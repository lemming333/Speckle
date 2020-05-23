#include "CQuat.h"

CQuat::CQuat()
	: mQuat{1.0, 0.0, 0.0, 0.0}
{}

CQuat::CQuat(CQuat& quat)
	: mQuat{quat.mQuat[0], quat.mQuat[1], quat.mQuat[2], quat.mQuat[3]}
{}

CQuat::CQuat(double w, double i, double j, double k)
	: mQuat{w, i, j, k}
{}

CQuat::~CQuat()
{}

CQuat& CQuat::add(CQuat& value)
{
	CQuat result(value.mQuat[0] + mQuat[0], 
		value.mQuat[1] + mQuat[1], 
		value.mQuat[2] + mQuat[2], 
		value.mQuat[3] + mQuat[3]);
	return result;
}

ostream& operator<<(ostream& os, const CQuat& quat)
{
	os << "[" << quat.mQuat[0] << "," << quat.mQuat[1] << "i," << quat.mQuat[2] << "j," << quat.mQuat[3] << "k]";
	return os;
}
