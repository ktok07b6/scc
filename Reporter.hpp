#ifndef __REPORTER_HPP__
#define __REPORTER_HPP__

class Reporter
{
public:
	Reporter();
	void error(int code, ...);
	void warning(int code, ...);
	void info(int code, ...);

	int getLastError();
	void reset();
private:
	int errCode;
};

extern class Reporter Reporter;

#endif
