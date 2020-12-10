#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <queue>
#include <map>

using namespace std;

enum ValType {
	VINT, VREAL, VSTR, VERR
};

class Value {
	ValType T;
	int I;
	float R;
	string S;

public:
	Value() :
			T(VERR), I(0), R(0.0) {
	}
	Value(int vi) :
			T(VINT), I(vi) {
	}
	Value(float vr) :
			T(VREAL), I(0), R(vr) {
	}
	Value(string vs) :
			T(VSTR), I(0), R(0.0), S(vs) {
	}

	ValType GetType() const {
		return T;
	}
	bool IsErr() const {
		return T == VERR;
	}
	bool IsInt() const {
		return T == VINT;
	}
	bool IsStr() const {
		return T == VSTR;
	}
	bool IsReal() const {
		return T == VREAL;
	}

	int GetInt() const {
		if (IsInt())
			return I;
		throw "RUNTIME ERROR: Value not an integer";
	}
	string GetStr() const {
		if (IsStr())
			return S;
		throw "RUNTIME ERROR: Value not a string";
	}
	float GetReal() const {
		if (IsReal())
			return R;
		throw "RUNTIME ERROR: Value not an integer";
	}

	// add op to this
	Value operator+(const Value &op) const;

	// subtract op from this
	Value operator-(const Value &op) const;

	// multiply this by op
	Value operator*(const Value &op) const;

	// divide this by op
	Value operator/(const Value &op) const;

	friend ostream& operator<<(ostream &out, const Value &op) {
		if (op.IsInt())
			out << op.I;
		else if (op.IsStr())
			out << op.S;
		else if (op.IsReal())
			out << op.R;
		else
			out << "ERROR";
		return out;
	}
};

// add this with op
Value Value::operator+(const Value &op) const {
	if (GetType() == op.GetType()) {
		if (IsInt())
			return Value(I + op.GetInt());
		if (IsReal())
			return Value(R + op.GetReal());
	} else if (IsInt() && op.IsReal()) {
		return Value((float) GetInt() + op.GetReal());
	} else if (IsReal() && op.IsInt()) {
		return Value(GetReal() + (float) op.GetInt());
	} else {
		return Value();
	}
}

// subtract op from this
Value Value::operator-(const Value &op) const {
	if (GetType() == op.GetType()) {
		if (IsInt())
			return Value(I - op.GetInt());

		if (IsReal())
			return Value(R - op.GetReal());
	} else if (IsInt() && op.IsReal()) {
		return Value((float) GetInt() - op.GetReal());

	} else if (IsReal() && op.IsInt()) {
		return Value(GetReal() - (float) op.GetInt());

	} else {
		return Value();
	}
}

// multiply this with op
Value Value::operator*(const Value &op) const {
	if (GetType() == op.GetType()) {
		if (IsInt())
			return Value(I * op.GetInt());
		if (IsReal())
			return Value(R * op.GetReal());
	} else if (IsInt() && op.IsReal()) {

		return Value((float) GetInt() * op.GetReal());
	} else if (IsReal() && op.IsInt()) {
		return Value(GetReal() * (float) op.GetInt());
	} else {
		return Value();
	}
}

// divide this by op
Value Value::operator/(const Value &op) const {
	if (GetType() == op.GetType()) {
		if (IsInt())
			return Value(I / op.GetInt());
		if (IsReal())
			return Value(R / op.GetReal());
	} else if (IsInt() && op.IsReal()) {

		return Value((float) GetInt() / op.GetReal());
	} else if (IsReal() && op.IsInt()) {
		return Value(GetReal() / ((float) op.GetInt()));
	} else {
		return Value();
	}
}

map<string, Value> symbolTable; //symbol Table of variable names and Value objects
queue<Value> *ValQue; //declare a pointer variable to a queue of Value objects

#endif
