
#ifndef STACK_H
#define STACK_H

// this is the node class used to build up the LIFO stack
template <class Data>
class Node {

private:

	Data holdMe;
	Node *next;
	
public:

	/*****************************************/
	/** WHATEVER CODE YOU NEED TO ADD HERE!! */
	/*****************************************/

};

// a simple LIFO stack
template <class Data> 
class Stack {

	Node <Data> *head;

public:

	// destroys the stack
	~Stack () { /* your code here */ }

	// creates an empty stack
	Stack () { /* your code here */ }

	// adds pushMe to the top of the stack
	void push (Data) { /* your code here */ }

	// return true if there are not any items in the stack
	bool isEmpty () { /* replace with your code */ return true; }

	// pops the item on the top of the stack off, returning it...
	// if the stack is empty, the behavior is undefined
	Data pop () { /* replace with your code */ return Data ();  }
};

#endif
