#pragma once
#include <iostream>

using namespace std;

class CQuat
{
public:
	CQuat();
	~CQuat();
	CQuat(CQuat& quat);
	CQuat(double w, double i, double j, double k);
	CQuat add(CQuat& value);
	ostream& operator<<(ostream& in);
	friend ostream& operator<<(ostream& os, const CQuat& dt);
private:
	double mQuat[4];
};

ostream& operator<<(ostream& os, const CQuat& dt);
