#ifndef __USER_H_

enum msg_level_t {
	LVL_DEBUG,
	LVL_INFO,
	LVL_MSG
};

/* This is a pure-virtual class (like java interface). */
class User {
public:
	virtual void Alert(const char *msg);
	virtual void Message(const char *msg, msg_level_t level = LVL_MSG);
};

#endif
