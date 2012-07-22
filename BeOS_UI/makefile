CXXFLAGS=-g -I ../Utils -I ../BeOS_UI -I ../Network -I. -I.. -I../Engine
no: engine HTMLView.o LinkBarView.o NOImage.o NOWindow.o PrefWindow.o Settings.o StatusBarView.o ToolBarView.o ToolView.o gr.o
	g++ -o no ../*/*.o ../../js*/src/*.o -lbe -lnetwork -ltranslation -ltextencoding -ltracker
	xres -o $@ ../NetOptimist.rsrc
	mimeset -f $@

engine:
	( cd ../Engine ; make )
