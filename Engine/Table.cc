#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#ifdef __BEOS__
#include <support/Debug.h>
#endif

#include "Table.h"
#include "DocWalker.h"
#include "Frame.h"
#include "EventCodes.h"
#include "Style.h"
#include "traces.h"

// XXX we must seriously reduce this and have dynamic values !
static const int MAXCOL = 40;
static const int MAXLINES = 700;


/*
	1) the table elem manages border width
	2) the table elem manages cell spacing
	3) the TD (!!!) manages the cell padding because is must draw the background.
*/


// -----------------------------------------------
class TableDim {
	// This is a simple (stupid) table dimension representation
	// likely to be rewritten, any volunter ?
	// It only accepts 50 columns and 300 lines
	struct Col {
		int min; // minimal column width - the size of the smallest element
		int max; // maximal column width - the sum of the size of all elements
		int req; // width required by html source
		int eff; // the resuling width
		bool req_abs; // req size in "pixels" or in "%"
		bool span;	// some cells in this col spans on a col on the right.
				// This mean that we should not trust min/max
	};
	int nb_col;
	int nb_lines;
	Col cols[MAXCOL];
	int colspans[MAXLINES][MAXCOL];
	int rowspans[MAXLINES][MAXCOL];
	int tab_eff;
	int m_tab_req;
	int line_height[MAXLINES];
	TableDocElem *table;
public:
	TableDim(TableDocElem * container) {
		table = container;
		reinit();
		for (int i=0;i<MAXLINES;i++) {
			for (int j=0;j<MAXCOL;j++) {
				colspans[i][j]=-1; // This mean each cell belongs to itself (ie no row/col span by default)
				rowspans[i][j]=-1;
			}
		}
	}
	void reinit() {
		nb_col = 0;
		nb_lines = 0;
		m_tab_req = -1;		// No requested width by default.
		line_height[nb_lines]=0;
		for (int i=0;i<MAXCOL;i++) {
			cols[i].min=0;
			cols[i].max=0;
			cols[i].req=-1;
			cols[i].eff=0;
		}
	}
	void add_line() {
		assert(nb_lines<MAXLINES-1);
		line_height[nb_lines]=0;
		nb_lines++;
	}
	bool infoLine(int line, int height) {
		assert(line<nb_lines);
		trace(DEBUG_TABLE_RENDER) printf("TableDim::infoLine : line %d - prev H : %d / new H %d\n",line,line_height[line],height);
		if (line_height[line] < height) {
			line_height[line] = height;
			return true;
		}
		return false;
	}
	int lineHeight(int line) {
		assert(line<nb_lines);
		return line_height[line];
	}
	void info_cell(int line, int col, int colspan, int rowspan, int minW, int maxW, int askedWidth, bool asked_percent);
	void update_col(int col, int minW);

	void Tags2Coord(int nbtr, int nbtd, int *line, int *col);
	int colSpan(int line, int col) {
		int j=col;
		while(j<nb_col && colspans[line][j]==col)
			j++;
		return j-col;
	}
	int cellHeight(int line, int col) {
		int t = line_height[line];
		for (int j=line+1; j<nb_lines && (rowspans[j][col]==line && colspans[j][col]==col); j++) {
			/* when cells spans */
			t += line_height[j];
			t += (table->borderWidth>0);
			t += table->cellSpacingWidth;
		}
		return t;
	}
	int cellWidth(int line, int col) {
		int t = 0;
		t += cols[col].eff;
		for (int j=col+1; j<nb_col && (rowspans[line][j]==line && colspans[line][j]==col); j++) {
			/* when cells spans */
			t += cols[j].eff;
			t += (table->borderWidth>0);
			t += table->cellSpacingWidth;
		}
		return t;
	}
	int colWidth(int col) {
		return cols[col].eff;
	}
	void place(int tab_req = -1);
	void print();
	int totalHeight() {
		int total = 0;
		for (int i=0;i<nb_lines;i++) {
			total += lineHeight(i);
		}
		if (nb_lines>1) {
			total += ((table->borderWidth>0) + table->cellSpacingWidth) * (nb_lines-1);
		}
		return total;
	}
	int totalWidth() {
		int t = 0;
		for (int j=0;j<nb_col;j++) {
			t += cols[j].eff;
		}
		if (nb_col>1) {
			t += ((table->borderWidth>0) + table->cellSpacingWidth) * (nb_col-1);
		}
		return t;
	}
	int NbLines() const { return nb_lines; }
	int NbCols() const { return nb_col; }
};

void TableDim::place(int tab_req) {
	int tab_max;
	if (tab_req>0) {	// Save requested width for table but do not reset previous value.
		m_tab_req = tab_req;
	}

	// Computes the min values
	for (int c=0;c<nb_lines;c++) {
		for (int j=0;j<nb_col;j++) {
			cols[j].eff = cols[j].min;
		}
	}

	// If the table can grow, dispatch on each columns
	tab_max = table->constraint->Width();
	tab_max -= 2 * table->borderWidth;
	/* This is now taken into account in totalWidth()...
	if (nb_col>1) {
		tab_max -= (nb_col-1) * ((table->borderWidth>0) + table->cellSpacingWidth);
	}
	*/

	trace(DEBUG_TABLE_RENDER) {
		printf("Table req width = %d\n", m_tab_req);
	}
	if (m_tab_req>0 && m_tab_req<tab_max) {
		tab_max = m_tab_req;
	}

	/* Now we will increase the columns sizes
	   I don't see any "perfect way"
	   Rules applied here : we will increase them 
	   	- up to the requested size (all cells)
	   and then, if still room to grow
	   	- to the max size for cell containing many items (maxW > 2*minW)
	   	- then even bigger if max>min (ie cells that contains more then one item) [ humm, what about cells containing table ? ]
	*/

	int increase = tab_max - totalWidth();
	int maxInc = min(10, nb_col>0 ? (increase+nb_col-1)/nb_col : 1);
	for (int level=0; level<14 && increase>0; level++) {
		bool canIncrease = true;
		while (canIncrease && increase>0) {
			canIncrease = false;
			for (int j=0;j<nb_col;j++) {
				bool cellShouldGrow;
#define EXPLICIT_WIDTH (cols[j].req>=0 && cols[j].req_abs)
				switch(level) {
				case 0:
					// increase the cells width up to the requested size
					cellShouldGrow = cols[j].eff<cols[j].req && cols[j].eff<cols[j].max;
					break;
				case 1:
					// increase the cells width up to the requested size even if beyong cell max
					// we trust absolute req values more than relative (percentage) values
					cellShouldGrow = cols[j].req_abs && cols[j].eff<cols[j].req;
					break;
/* This one breaks apple.com 
				case 2:
					// increase the cells width up to the requested size even if beyong cell max
					cellShouldGrow = cols[j].eff<cols[j].req;
					break;
*/
				case 3:
					// increase the cells width for cells "just bigger" than the min (+20)
					// so that there are no "stupid line-breaks"
					cellShouldGrow = cols[j].eff<cols[j].max && cols[j].max <= 20+cols[j].min;
					break;
				case 4:
					// increase the cells width up to the max size of "heavy cells" (max > 3*min)
					cellShouldGrow = !EXPLICIT_WIDTH && cols[j].eff<cols[j].max && cols[j].max > 3*cols[j].min;
					break;
				case 5:
					// increase the cells width up to the max size
					cellShouldGrow = !EXPLICIT_WIDTH && cols[j].eff<cols[j].max;
					break;
				case 6:
					// We will now increase the table even beyong the max cell width
					cellShouldGrow = !EXPLICIT_WIDTH && m_tab_req >0 && cols[j].max > 2*cols[j].min;
					break;
				case 7:
					cellShouldGrow = !EXPLICIT_WIDTH && m_tab_req >0 && (cols[j].max > cols[j].min || cols[j].span);
					break;
				case 8:
					// If a width value for table is set in html code, we must
					// still increase the cell (whatever the content width is)
					// But while doing so, we prefer already big cells
					cellShouldGrow = m_tab_req>0 && cols[j].eff<cols[j].max;
					break;
				case 9:
					// Some tables contain 1 cell and table size is requested
					cellShouldGrow = m_tab_req >0 &&
						(cols[j].eff>cols[j].max ||
						 cols[j].req>0 && cols[j].eff>cols[j].req );
					break;
				case 10:
					// Some tables contain 1 cell and table size is requested
					cellShouldGrow = !EXPLICIT_WIDTH && (cols[j].eff!=cols[j].min || cols[j].eff!=cols[j].max) && m_tab_req >0;
					break;
				case 11:
					// Some tables contain 1 cell and table size is requested
					cellShouldGrow = (cols[j].eff!=cols[j].min || cols[j].eff!=cols[j].max) && m_tab_req >0;
					break;
				case 12:
					// Some tables contain 1 cell and table size is requested
					cellShouldGrow = m_tab_req >0;
					break;
				default:
					cellShouldGrow = false;
				}
				if (cellShouldGrow && increase>0) {
					int inc = min(increase, maxInc);
					if (cols[j].max>cols[j].eff)
						inc = min(inc, cols[j].max-cols[j].eff);
					if (cols[j].req>cols[j].eff)
						inc = min(inc, cols[j].req-cols[j].eff);
					trace(DEBUG_TABLE_RENDER) {
						if ((cols[j].eff>cols[j].req && cols[j].req>=0) || cols[j].max==1 )
							printf("INCREASE : stage %d min=%d, max=%d, eff=%d, req=%d, (+%d)\n",
								level, cols[j].min, cols[j].max, cols[j].eff, cols[j].req, inc);
					}
					cols[j].eff += inc;
					increase-=inc;
					canIncrease = true;
				} else {
					trace(DEBUG_TABLE_RENDER) {
						if (level>8)
							printf("NO INCREASE : stage %d min=%d, max=%d, eff=%d, req=%d\n",
								level, cols[j].min, cols[j].max, cols[j].eff, cols[j].req);
					}
				}
			}
		}
	}

	trace(DEBUG_TABLE_RENDER) {
		print();
		for (int j=0;j<nb_col;j++) {
			if (cols[j].max < cols[j].min) {
				fprintf(stdout, "Ooops : for col %d. max(%d) < min(%d)\n", j, cols[j].max, cols[j].min);
			}
		}
	}
}

void TableDim::print() {
	fprintf(stdout, "TABLE id=%d col %d * line %d\n",table->id,nb_col,nb_lines);
	for (int c=0;c<nb_col;c++) {
		fprintf(stdout, "\t%d<<%d",cols[c].min,cols[c].max);
	}
	fprintf(stdout, "\n");
	for (int i=0;i<nb_lines;i++) {
		if (line_height[i]==0) break;
		for (int j=0;j<nb_col;j++) {
			fprintf(stdout, "\tH%d * L%dR%d", line_height[i], cols[j].eff, cols[j].req);
			for (int k=0;k<colSpan(i,j)-1; k++) 
				fprintf(stdout, "\t_%d_", colSpan(i,j));
		}
		fprintf(stdout, "\n");
	}
	for (int k=0;k<nb_lines;k++) {
		for (int l=0;l<nb_col;l++) {
			fprintf(stdout, "\t(%d,%d)", rowspans[k][l], colspans[k][l]);
		}
		fprintf(stdout, "\n");
	}
	fprintf(stdout, "\tConstraint Width = %d\n", table->constraint->Width());
	fprintf(stdout, "\tcellPadding = %d\n", table->cellPaddingWidth);
	fprintf(stdout, "\tcellSpacing = %d\n", table->cellSpacingWidth);
}

void TableDim::Tags2Coord(int nbtr, int nbtd, int *line, int *col) {
	*line = nbtr;		// ok this one is easy :-)

	*col = 0;
	while (*col<nb_col && (rowspans[*line][*col]!=-1 || colspans[*line][*col]!=-1))
		(*col)++;
	// We are on the first master cell
	for (int i=0; i<nbtd; i++) {
		(*col)++;
		// Find the next master cell
		while (*col<nb_col && (rowspans[*line][*col]!=-1 || colspans[*line][*col]!=-1))
			(*col)++;
	}
}

void TableDim::info_cell(int line, int col, int colspan, int rowspan, int minW, int maxW, int askedWidth, bool asked_percent) {
	if (col+colspan>nb_col) {
		// a new column is found
		for (int k = nb_col; k<col+colspan; k++) {
			// default values for spaned cols
			cols[k].span = false;
			cols[k].req = -1;
			cols[k].req_abs = 0;
		}
		nb_col = col+colspan;
		assert(col<MAXCOL);
		cols[col].span = false;
		cols[col].req = askedWidth;
		cols[col].req_abs = !asked_percent;
		if (colspan==1) {
			cols[col].min = minW;
			cols[col].max = max(minW,maxW);
		}
		assert(cols[col].max>=cols[col].min);
	} else if (colspan==1) {
		if (askedWidth>cols[col].req) {
			cols[col].req = askedWidth;
		}
		if (maxW>cols[col].max) {
			cols[col].max = maxW;
		}
		if (minW>cols[col].min) {
			cols[col].min = minW;
			if (minW>cols[col].max) {
				cols[col].max = minW; // Max should not be smaller the min
			}
		}
		assert(cols[col].max>=cols[col].min);
	}

	/* Flags the rowspaned and colspaned cell with the coord */
	for (int j = 0; j+line<MAXLINES && j<rowspan; j++) {
		for (int i = 0; i+col<nb_col && i<colspan; i++) {
			colspans[line + j][col + i]=col;
			rowspans[line + j][col + i]=line;
		}
	}
	if (colspan>1) cols[col].span=true;
	// this overwrite the above loop
	colspans[line][col]=-1; // This mean this cell belongs to itself
	rowspans[line][col]=-1;
}

void TableDim::update_col(int col, int minW) {
	if (minW>cols[col].min) {
		cols[col].min = minW;
		if (minW>cols[col].max) {
			cols[col].max = minW; // Max should not be smaller the min
		}
	}
	assert(cols[col].max>=cols[col].min);
}

// -----------------------------------------------
void TableDocElem::updateCol(int line, int col, int size) {
	int cellWidth = dims->cellWidth(line,col);
	if (cellWidth<size) {
		trace(DEBUG_RENDER)
			fprintf(stdout, "Update for col %d,%d w=%d old=%d\n", line, col, size, cellWidth);
		dims->update_col(col, size - cellWidth + dims->colWidth(col));
				// this is for handling col spans XXX and is not good at all !
	}
}

void TableDocElem::updateLine(int line, int col, int colheight) {
	int lineHeight = colheight - dims->cellHeight(line,col) + dims->lineHeight(line);
	if (dims->infoLine(line, lineHeight)) {
		dims->place();
		Msg(DRAW);
		ProcessAll();
	}
	constraint->nextY = max(constraint->nextY, y+h);
}

bool TableDocElem::update(int right, int top, int bottom) {
	if (top<y) {
		fprintf(stderr, "WARNING : Update for TD elem is above cell : %d<%d\n", top, y);
	}
	int realH = bottom - y + 1;
	int realW = right - x + 1;
	realH += borderWidth;
	realW += borderWidth;
	if (w<realW || h<realH) {
		trace(DEBUG_RENDER)
			fprintf(stdout, "Update for TABLE %d H%d>%d W%d>%d\n", id, realH, h, realW, w);
		/*
		constraint->nextLineY = max(constraint->nextLineY, y+h);
		constraint->nextLineY = max(constraint->nextLineY, bottom);
		constraint->lineH = max(constraint->lineH, h);
		constraint->lineH = max(constraint->lineH, realH);
		*/
		// XXX Hmm, shouldn't we DO something here ?
		return true;
	}
	return false;
}

void TableDocElem::Process(int evt) {
	switch(evt) {
		case DRAW:
			PlaceChildren();
			break;
	}
}

void TableDocElem::place() {
	TagAttr *attr;
	// Table placement algo.

	if (dims==NULL)
		dims = new TableDim(this);

	// LINE BREAK
	constraint->NewLine(m_style->Indent(), 0);

	x=constraint->nextX;
	y=constraint->nextY;

	int nbTR = 0;
	for (TR_List *tr_elem=contentList; tr_elem!=NULL; tr_elem=tr_elem->next) {
		TR_DocElem *nextTag = tr_elem->elem;
		if (nextTag) {
			int nbTD = 0;
			for (TR_List::TD_List *td_elem=tr_elem->cells; td_elem!=NULL; td_elem=td_elem->next) {
				TD_DocElem *nextTag_col = td_elem->elem;
				if (nextTag_col) { // tag <TD>
					if (nextTag_col->Included()) {
						int line,col;
						int minW = 0;
						int maxW = 0;
						int reqW = -1;
						bool reqWpercent = false;
						DocWalker walk(nextTag_col->Included());
						DocElem *iter_data;
						int cellWidth = 0;
						int contentH = 0;
						int TDlineH = 0;
						while ((iter_data = walk.Next())) {
							const Tag *te = NULL;
							int iterW = iter_data->fixedW;
#define USE_HACK 2
#if (USE_HACK==0)
							if (iter_data->fixedW == 0
								|| (!dynamic_cast<TableDocElem*>(iter_data)
									&& !dynamic_cast<TD_DocElem*>(iter_data)))
								walk.Feed(iter_data);
#elif (USE_HACK==1)
							if (dynamic_cast<TableDocElem*>(iter_data)
								|| dynamic_cast<TD_DocElem*>(iter_data))
								iterW = 0;
							walk.Feed(iter_data);
#elif (USE_HACK==2)
							walk.Feed(iter_data);
#endif
							TagDocElem *brTag = dynamic_cast<TagDocElem*>(iter_data);
							if (brTag)
								te= brTag->tag();
							if (iterW > minW)
								minW=iterW;
							trace(DEBUG_TABLE_RENDER) {
								if (te && te->toString()) {
									fprintf(stderr,
										"<TABLE %d/TD %d> content <%s> width %d => cellWidth %d\n",
										id, nextTag_col->id, te->toString(),
										iterW, cellWidth);
								}
							}
							if (te && te->info && te->info->vspace_before>=0 ) {
								if (cellWidth>maxW) 
									maxW = cellWidth;
								TDlineH += contentH;
								contentH = 0;
								cellWidth = 0;
							}
							cellWidth += iterW;
							contentH = max(contentH,iter_data->h);
							if (te && te->info && te->info->vspace_after>=0 ) {
								if (cellWidth>maxW) 
									maxW = cellWidth;
								TDlineH += contentH;
								contentH = 0;
								cellWidth = 0;
							}
						}
						TDlineH += contentH;
						if (cellWidth>maxW) 
							maxW = cellWidth;

						dims->Tags2Coord(nbTR, nbTD, &line, &col);
						nextTag_col->col  = col;
						nextTag_col->line = line;

						for (attr = nextTag_col->AttributeList(); attr!=NULL; attr=attr->Next()) {
							if (attr->ReadDim("WIDTH",&reqW,&reqWpercent, constraint->Width())) {
								trace(DEBUG_TABLE_RENDER)
									printf("Found WIDTH=%d in row %d\n",reqW,nbTR);
							}
						}

						if (minW < nextTag_col->includedConstraint->MinW()) {
							minW = nextTag_col->includedConstraint->MinW();
						}
					// DEBUG
						else if (minW > nextTag_col->includedConstraint->MinW()) {
							fprintf(stderr, "<TABLE> computed cell min = %d, registered %d\n",
								minW, nextTag_col->includedConstraint->MinW());
						}
					// DEBUG
						if (maxW<minW) {
							fprintf(stderr, "<TABLE> computed min(%d) > max(%d)\n", minW, maxW);
							maxW = minW;
						}
						
						maxW += cellPaddingWidth * 2;
						minW += cellPaddingWidth * 2;
						
						dims->info_cell(line, col, nextTag_col->colspan, nextTag_col->rowspan, minW,maxW,reqW,reqWpercent);
					}
					nbTD++;
				}
			}
			dims->add_line();
			nbTR++;
		}
	}
	int reqW_for_table = -1;
	for (attr = list; attr!=NULL; attr=attr->Next()) {
		/* XXX this should be done before computing cellW (???) */
		bool dummy;
		if (attr->ReadDim("WIDTH",&reqW_for_table,&dummy, constraint->Width())) {
			trace(DEBUG_TABLE_RENDER)
				printf("Found WIDTH=%d for table id=%d\n",reqW_for_table,id);
		}
	}
	dims->place(reqW_for_table);

	computeSize();
	
	includedConstraint->Init(y, x, x+w);
	/*
	includedConstraint->nextLineY = y + h;
	*/
	includedConstraint->lineH = h;
				// We imediately jump at the end of the table as cells are place
				// manually. In fact there souldn't be any data not included in TR and TD

	Msg(DRAW);
	ProcessAll();

	/*
	constraint->nextLineY = max(constraint->nextLineY, y+h);
	*/
	constraint->lineH = max(constraint->lineH, h);
	constraint->NewLine(m_style->Indent(), 0);

	printPosition("table_place");
}

TableDocElem::TableDocElem(Tag *tag, TagAttr* attrList) : TagDocElem(tag, attrList) {
	dims = NULL;
	borderWidth = 0;
	cellPaddingWidth = 0;
	cellSpacingWidth = 0;
	contentList = NULL;
	attr_align = al_left;
	m_attr_height = -1;
	includedConstraint = new Constraint(this);
}

TableDocElem::~TableDocElem() {
	delete dims;
	TR_List *tr_elem=contentList;
	while(tr_elem!=NULL) {
		TR_DocElem *nextTR = tr_elem->elem;
		if (nextTR) {
			// for each line...
			TR_List::TD_List *td_elem=tr_elem->cells;
			while (td_elem!=NULL) {
				TR_List::TD_List *cur=td_elem;
				td_elem=td_elem->next;
				delete cur;
			}
		}
		TR_List *tr_cur = tr_elem;
		tr_elem=tr_elem->next;
		delete tr_cur;
	}
}

void TableDocElem::RegisterLine(TR_DocElem *tr) {
	TR_List *newList = new TR_List();
	newList->next=NULL;
	newList->elem=tr;
	newList->cells=NULL;
	if (contentList==NULL) {
		contentList=newList;
	} else {
		TR_List *last=contentList;
		while (last->next)
			last=last->next;
		last->next = newList;
	}
}

void TableDocElem::RegisterCell(TD_DocElem *td) {
	TR_List::TD_List *newList = new TR_List::TD_List;
	newList->next=NULL;
	newList->elem=td;

	TR_List *lastLine=contentList;
	while (lastLine->next)
		lastLine=lastLine->next;

	if (lastLine->cells==NULL) {
		lastLine->cells=newList;
	} else {
		TR_List::TD_List *lastCell=lastLine->cells;
		while (lastCell->next)
			lastCell = lastCell->next;
		lastCell->next = newList;
	}
}

void TableDocElem::PlaceChildren() {
	int cell_y = y + borderWidth;
	int nbTR = 0;
	for (TR_List *tr_elem=contentList; tr_elem!=NULL; tr_elem=tr_elem->next) {
		TR_DocElem *nextTR = tr_elem->elem;
		if (nextTR) {
			// for each line...
			int cell_x = x + borderWidth;
			int nbTD = 0;
			int nbCol = 0;

			nextTR->x = cell_x;
			nextTR->y = cell_y;
			nextTR->h = dims->lineHeight(nbTR);
			nextTR->w = w - 2*borderWidth;
			for (TR_List::TD_List *td_elem=tr_elem->cells; td_elem!=NULL; td_elem=td_elem->next) {
				TD_DocElem *nextTD = td_elem->elem;
				if (nextTD) {
					// set geometry for each colunm
					int line, col;

					dims->Tags2Coord(nbTR, nbTD, &line, &col);

#if 0
	/* Does not work because the test doesn't distinguish rowspan from colspan */
	/* XXX remove the symbol nbCol, well in fact no */
					while (nbCol<col) {
						// This happens because a <TD> rowspaned on this line
						// We must add the width for all skiped columns
						cell_x += dims->colWidth(nbCol) + (borderWidth>0) + cellSpacingWidth;
						nbCol++;
					}
#endif
					if (nbCol==col) {
						// This is the simple case where there is no row/col span on the line 
						// since the previous cell :
						// In this case we can do it incrementally
						// ... well nothing to do : cell_x is up to date
					} else {
						cell_x = x + borderWidth;
						for(int k=0; k<col; k++)
							cell_x += dims->colWidth(k) + (borderWidth>0) + cellSpacingWidth;
						nbCol = col;
					}


					nextTD->x = cell_x;
					nextTD->y = cell_y;
					nextTD->w = dims->cellWidth(line, col);
					nextTD->h = dims->cellHeight(line, col);
					nextTD->place();

					cell_x += nextTD->w + (borderWidth>0) + cellSpacingWidth;
					
					nbTD++;
					nbCol++;
				}
			}
			cell_y += dims->lineHeight(nbTR) + (borderWidth>0) + cellSpacingWidth;
			nbTR++;
		}
	}
	computeSize();
}

void TableDocElem::computeSize() {
	w = dims->totalWidth() + 2 * borderWidth;
	h = dims->totalHeight() + 2 * borderWidth;
	if (m_attr_height>h)
		h = m_attr_height;
}

void TableDocElem::geometry(HTMLFrame *) {
	const char *attr_value;
	m_includedStyle = m_style->clone(id);
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		rgb_color color; 
		if (iter->ReadColor("color",&color))
			m_includedStyle->SetColor(color);
		if (iter->ReadColor("bgcolor",&color))
			m_includedStyle->SetBGColor(color);
		iter->ReadInt("border",&borderWidth);
		if (iter->ReadStrp(NULL, &attr_value) && attr_value) {
			if (!strcasecmp(attr_value, "border")) borderWidth=1;
		}
		iter->ReadInt("cellpadding",&cellPaddingWidth);
		iter->ReadInt("cellspacing",&cellSpacingWidth);
		iter->ReadInt("height",&m_attr_height);
		iter->ReadAlignment("ALIGN",&attr_align);
	}
	trace(DEBUG_FORCETABLEBORDER) {
		//cellPaddingWidth=0;
		cellSpacingWidth=2;
		borderWidth=2;
	}
}

void TableDocElem::dynamicGeometry(HTMLFrame *view) {
	int minW = borderWidth * 2;
	for (TR_List *tr_elem=contentList; tr_elem!=NULL; tr_elem=tr_elem->next) {
		TR_DocElem *nextTR = tr_elem->elem;
		if (nextTR) {
			int rowMinW = 0;
			// for each line...
			for (TR_List::TD_List *td_elem=tr_elem->cells; td_elem!=NULL; td_elem=td_elem->next) {
				TD_DocElem *nextTD = td_elem->elem;
				if (nextTD) {
					rowMinW += nextTD->includedConstraint->minW;
				}
				if (td_elem->next) {
					// Add border + spacing
					rowMinW += cellSpacingWidth + (borderWidth>0);
				}
			}
			minW = max(minW, rowMinW);
		}
	}
	constraint->RegisterWidth(minW);
}

void TableDocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	view->FillRect(x,y,w,h,m_includedStyle);
	if (borderWidth>0)
		view->DrawBorder3D(x,y,w,h,m_includedStyle, borderWidth);
	trace(DEBUG_FORCETABLEBORDER) {
		view->DrawBorder3D(x,y,w,h,m_includedStyle, borderWidth);
	}
	TagDocElem::draw(view);
}

void TableDocElem::initPlacement() {
	TagDocElem::initPlacement();
	if (dims)
		dims->reinit();
}

// -----------------------------------------------

void TR_DocElem::RelationSet(HTMLFrame *view) {
	TagDocElem::RelationSet(view);
	DocElem *p = Predecessor();
	while(p != NULL) {
		container = dynamic_cast<TableDocElem*>(p);
		if (container) {
			break;
		}
		p = p->Predecessor();
	}
	if (container) {
		container->RegisterLine(this);
	} else {
		fprintf(stderr, "ERROR : did not Found container TABLE\n");
	}
}

void TR_DocElem::geometry(HTMLFrame */*view*/) {
	m_includedStyle = m_style->clone(id);
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		rgb_color color; 
		if (iter->ReadColor("color",&color))
			m_includedStyle->SetColor(color);
		if (iter->ReadColor("bgcolor",&color))
			m_includedStyle->SetBGColor(color);
	}
}

// -----------------------------------------------

void TD_DocElem::RelationSet(HTMLFrame *view) {
	TagDocElem::RelationSet(view);
	DocElem *p = Predecessor();
	while(p != NULL) {
		TR_DocElem *TR_container = dynamic_cast<TR_DocElem*>(p);
		if (TR_container) {
			container = TR_container->container;
			break;
		}
		p = p->Predecessor();
	}
	if (container) {
		container->RegisterCell(this);
	} else {
		fprintf(stderr, "ERROR : did not Found container TABLE\n");
	}
}

void TD_DocElem::geometry(HTMLFrame *) {
	m_includedStyle = m_style->clone(id);
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		rgb_color color; 
		if (iter->ReadColor("color",&color))
			m_includedStyle->SetColor(color);
		if (iter->ReadColor("bgcolor",&color))
			m_includedStyle->SetBGColor(color);
		iter->ReadInt("colspan",&colspan);
		iter->ReadInt("rowspan",&rowspan);
		iter->ReadInt("height",&m_attr_height);
		iter->ReadAlignment("ALIGN",&attr_align);
	}
	if (container) {
		w = container->cellPaddingWidth * 2;
		h = container->cellPaddingWidth * 2;
		if (m_attr_height>h)
			h = m_attr_height;
	}
}

void TD_DocElem::place() {
	if (x == -1 && y == -1) {
		fprintf(stderr, "TD_DocElem(%d) has not been placed by container\n", id);
		if (constraint) {
			constraint->NextPosition(&x, &y, w, h, m_style->Indent()); // Ok this is bad but better than nothing
		}
	}
	int border = 0;
	if (container) {
		border = container->cellPaddingWidth;
	}
	/*
	includedConstraint->Init(	max(y + border, includedConstraint->nextY),
					x + border,
					x + w - border,
					max(x + border, includedConstraint->nextX),
					max(y + border, includedConstraint->nextLineY));
	*/
	includedConstraint->minX = x + border;
	includedConstraint->maxX = x + w - border - 1;
	includedConstraint->nextX = max(x + border, includedConstraint->nextX);
	includedConstraint->nextY = max(y + border, includedConstraint->nextY);
	includedConstraint->Check();
	/*
	includedConstraint->nextLineY = max(y + border, includedConstraint->nextLineY);
	*/
	printPosition("TD_place");
}

void TD_DocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	
	view->FillRect(x,y,w,h, m_includedStyle);
	if (!constraint) {
		fprintf(stderr, "Error : no container for TD tagid %d\n", id);
	}
	if (container && container->borderWidth>0) {
		// Draw border
		view->DrawRect(x-1,y-1,w+2,h+2, m_includedStyle);
	}
	trace(DEBUG_FORCETABLEBORDER) {
		view->DrawRect(x-1,y-1,w+2,h+2, m_includedStyle);
	}
	TagDocElem::draw(view);
}

bool TD_DocElem::update(int right, int top, int bottom) {
	if (top<y) {
		fprintf(stderr, "WARNING : Update for TD elem is above cell : %d<%d\n", top, y);
	}
	int realH = bottom - y + 1;
	int realW = right - x + 1;
	if (container) {
		realH += container->cellPaddingWidth;
		realW += container->cellPaddingWidth;
		if (m_attr_height>realH)
			realH = m_attr_height;
		if (w<realW) {
			trace(DEBUG_TABLE_RENDER) {
				fprintf(stdout, "Update for TD %d W %d was %d\n", id, realW, w);
				fprintf(stdout, "\tcellPadding = %d\n", container->cellPaddingWidth);
				fprintf(stdout, "\tcellSpacing = %d\n", container->cellSpacingWidth);
			}
			container->updateCol(line, col,realW);
		}
		if (h<realH) {
			trace(DEBUG_TABLE_RENDER)
				fprintf(stdout, "Update for TD %d H %d was %d\n", id, realH, h);
			container->updateLine(line,col,realH);
		}
		return true;
	} else {
		fprintf(stderr, "ERROR : no container for TD\n");
	}
	return false;
}

