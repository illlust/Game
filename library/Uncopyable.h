#pragma once

//////////////////////////////////////////////////////////////////////////

class Uncopyable
{
protected:
	Uncopyable() {}
	virtual ~Uncopyable() {}

private:
	Uncopyable(const Uncopyable&);
	Uncopyable& operator =(const Uncopyable&);
};

//////////////////////////////////////////////////////////////////////////

class Abstract
{
private:
	Abstract();
	virtual ~Abstract();

	Abstract(const Abstract&);
	Abstract& operator =(const Abstract&);
};
