// ===============================================
// Copyright @ Fan Mei 2018
// Author : Fan Mei
// Date : 2018-07-07 18:00:00
// Description : Node Editor
// ===============================================
#ifndef _NODE_H_
#define _NODE_H_

#include <typeinfo>
#include <iostream>
#include <vector>

#define MAX_CONNECTIONS 99
#define SOURCE_NODE 1
#define OPERATOR_NODE 2
#define SINK_NODE 3
#define UNKNOWN_NODE 99

#define INTEGER_DATA 1
#define FLOAT_DATA 2
#define STRING_DATA 3
#define UNKNOWN_TYPE_DATA 99

using namespace std;

//                                      Boost::Any
//-------------------------------------------------------------------------------------------
class any
{
public:
	any() : content(0)
	{
	}
	template<typename ValueType>
	any(const ValueType& value) : content(new holder<ValueType>(value))
	{
	}
	any(const any & other)
		: content(other.content ? other.content->clone() : 0)
	{
	}

	~any()
	{
		delete content;
	}

public: // modifiers
	any & swap(any & rhs)
	{
		std::swap(content, rhs.content);
		return *this;
	}
	template<typename ValueType>
	any & operator=(const ValueType & rhs)
	{
		any(rhs).swap(*this);
		return *this;
	}

	any & operator=(any rhs)
	{
		any(rhs).swap(*this);
		return *this;
	}

public: // queries
	bool empty() const
	{
		return !content;
	}

	void clear()
	{
		any().swap(*this);
	}

	const std::type_info & type() const
	{
		return content ? content->type() : typeid(void);
	}

	class placeholder
	{
	public: // structors
		virtual ~placeholder()
		{
		}
	public: // queries
		virtual const std::type_info & type() const = 0;
		virtual placeholder * clone() const = 0;
	};
public:
	template<typename ValueType>
	class holder : public placeholder
	{
	public:
		holder(const ValueType& value) : held(value)
		{
		}
		virtual const std::type_info& type() const
		{
			return typeid(ValueType);
		}

		virtual placeholder * clone() const
		{
			return new holder(held);
		}
	public: // representation
		ValueType held;
	private: // intentionally left unimplemented
		holder & operator=(const holder & held)
		{
			return holder(held);
		};
	};
	placeholder* content;
};

inline void swap(any & lhs, any & rhs)
{
	lhs.swap(rhs);
}

class  bad_any_cast : public std::bad_cast
{
public:
	virtual const char * what() const
	{
		return "bad_any_cast: "
			"failed conversion using any_cast";
	}
};

template<typename ValueType>
ValueType * any_cast(any * operand)
{
	return operand &&
#ifdef ANY_TYPE_ID_NAME
		std::strcmp(operand->type().name(), typeid(ValueType).name()) == 0
#else
		operand->type() == typeid(ValueType)
#endif
		? &static_cast<any::holder<ValueType> *>(operand->content)->held
		: 0;
}

template<typename ValueType>
inline const ValueType * any_cast(const any * operand)
{
	return any_cast<ValueType>(const_cast<any *>(operand));
}

template<typename ValueType>
ValueType any_cast(any& value)
{
	return static_cast<any::holder<ValueType>*>(value.content)->held;
}
template<typename ValueType>
ValueType any_cast(const any& value)
{
	return any_cast<ValueType>(const_cast<any &>(value));
}

//-------------------------------------------------------------------------------------------

class Node
{
public:
	Node() {}
	virtual ~Node() {}

	//Node *nextNodes[MAX_CONNECTIONS] = {NULL};
	Node *previousNodes[MAX_CONNECTIONS] = { NULL };

	any DataFlow;

	int nodeType = 99;
	int dataType = 99;
	int maxConnections = 99;
	bool previousNodesCooked = false;
	bool isCooked = false;

	virtual void function() {}

	// Check All Previous Nodes Exist.
	bool checkAllPerviousNodesExist() {
		bool isExist = true;
		if (nodeType == SOURCE_NODE) {
			isExist = true;
		} else {
			for (int i = 0; i < maxConnections; i++) {
				if (previousNodes[i] == NULL) {
					isExist = false;
				}

			}
		}
		return isExist;
	}

	// Check All Previous Nodes Cooked.
	void checkAllPerviousNodesCooked() {
		for (int i = 0; i < maxConnections; i++) {
			if (previousNodes[i] == NULL) { previousNodesCooked = false; break; }
			if (previousNodes[i]->isCooked == true) {
				previousNodesCooked = true;
			} else {
				previousNodesCooked = false;
			}
		}
	}

	// Cook.
	bool cook() {
		checkAllPerviousNodesCooked();
		if (!isCooked && previousNodesCooked) {
			function();
			isCooked = true;
		} else {
			isCooked = false;
		}
		return isCooked;
	}
};

/*
class IntegerNode : public Node
{
public:
	IntegerNode() {}
	virtual ~IntegerNode() {}
	//IntegerNode *previousNodes[MAX_CONNECTIONS] = { NULL };
};

class FloatNode : public Node
{
public:
	FloatNode() {}
	virtual ~FloatNode() {}
	//FloatNode *previousNodes[MAX_CONNECTIONS] = { NULL };
};
*/

//-------------------------------------------------------------------------------------------

class intSource_Node : public Node
{
public:
	intSource_Node() {
		intSource_Node::nodeType = SOURCE_NODE;
		intSource_Node::dataType = INTEGER_DATA;
		intSource_Node::isCooked = true;
		intSource_Node::DataFlow = any(int(0));
	}
	intSource_Node(int value) {
		intSource_Node::nodeType = SOURCE_NODE;
		intSource_Node::dataType = INTEGER_DATA;
		intSource_Node::isCooked = true;
		intSource_Node::DataFlow = any(int(value));
	}
};

class intAdd_Node : public Node
{
public:
	intAdd_Node() {
		intAdd_Node::nodeType = OPERATOR_NODE;
		intAdd_Node::dataType = INTEGER_DATA;
		intAdd_Node::maxConnections = 2;
		intAdd_Node::DataFlow = any(int(0));
	}
	virtual void function() override {
		int a = any_cast<int>(previousNodes[0]->DataFlow);
		int b = any_cast<int>(previousNodes[1]->DataFlow);
		intAdd_Node::DataFlow = any(int(a + b));
	}
};

class intPrint_Node : public Node
{
public:
	intPrint_Node() {
		intPrint_Node::nodeType = SINK_NODE;
		intPrint_Node::dataType = INTEGER_DATA;
		intPrint_Node::maxConnections = 1;
	}
	virtual void function() override {
		int v = any_cast<int>(intPrint_Node::previousNodes[0]->DataFlow);
		cout << "Print: " << v << "\n";
	}
};

class IntegerToFloat_Node : public Node
{
public:
	IntegerToFloat_Node() {
		IntegerToFloat_Node::nodeType = OPERATOR_NODE;
		IntegerToFloat_Node::dataType = FLOAT_DATA;
		IntegerToFloat_Node::maxConnections = 1;
		IntegerToFloat_Node::DataFlow = any(float(0.0));
	}
	virtual void function() override {
		float v = any_cast<int>(IntegerToFloat_Node::previousNodes[0]->DataFlow);
		IntegerToFloat_Node::DataFlow = any(float(v));
	}
};

//-------------------------------------------------------------------------------------------

static vector<any> nodes;
static int uncookednodes = 0;

static void Run() 
{
	for (int i = 0; i < nodes.size(); i++)
	{
		Node *node = any_cast<Node *>(nodes[i]);
		if (node->checkAllPerviousNodesExist() == false)
		{
			goto JMP_1;
		}

	}

	// Filter Source Nodes
	for (int i = 0; i < nodes.size(); i++)
	{
		Node *node = any_cast<Node *>(nodes[i]);
		if (node->isCooked == false)
		{
			uncookednodes += 1;
		}
	}

	cout << "uncookednodes :" << uncookednodes << "\n";

	// Cook Nodes
	while (uncookednodes > 0)
	{
		for (int i = 0; i < nodes.size(); i++)
		{
			Node *node = any_cast<Node *>(nodes[i]);
			if ((node->nodeType != SOURCE_NODE) && (node->isCooked == false))
			{
				if (node->cook())
				{
					uncookednodes -= 1;
				}
			}
		}
	}

	cout << "uncookednodes :" << uncookednodes << "\n";

JMP_1:
	cout << "Some Nodes Not Connected.\n";
	
}

#endif // !_NODE_H_

/*
EXAMPLE:

intSource_Node *a = new intSource_Node(10);
intSource_Node *b = new intSource_Node(30);
intSource_Node *c = new intSource_Node(50);

intAdd_Node *add_1 = new intAdd_Node();
intAdd_Node *add_2 = new intAdd_Node();

intPrint_Node *pr = new intPrint_Node();

add_1->previousNodes[0] = a;
add_1->previousNodes[1] = b;

add_2->previousNodes[0] = add_1;
add_2->previousNodes[1] = c;

pr->previousNodes[0] = add_2;

nodes.push_back(any(a));
nodes.push_back(any(b));
nodes.push_back(any(c));
nodes.push_back(any(add_1));
nodes.push_back(any(add_2));
nodes.push_back(any(pr));

Run();
*/