/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

#ifndef __QUEUE_H__
#define __QUEUE_H__

/*
使用定长数组充当队列，数组指针的head和tail表征队列的首元素、尾元素、是否为空、是否为满。
*/

/*
模板类的成员函数，如果使用了模板类型，则必须在类内定义。
"To konw why, let us look at the compilation process."
*/

#include "iostream"
#include "string.h"

using namespace std;

void testQueue();

template <class T>
class Queue{
public:
	Queue();
	Queue(const unsigned int depth);
	~Queue();

	int push(const T *pData);
	int pop(T *pData);
	int relinquish();
	int clear();
	bool isFull();
	bool isEmpty();
	unsigned int depth();
	unsigned int maxDepth();
	int setQueueDepth(unsigned int depth);
	
	void output();
	
private:
	unsigned int queueMaxDepth = 0;
	T *dataArray = NULL;
	T *pHead = NULL;
	T *pTail = NULL;
	unsigned int curDepth = 0;

	bool checkOK();
	void headIncrease();
	void headDecrease();
	void tailIncrease();
	void tailDecrease();
};

template <class T>
Queue<T>::Queue()
{
	dataArray = NULL;
	pHead = NULL;
	pTail = NULL;
	queueMaxDepth = 0;
	curDepth = 0;
}

template <class T>
Queue<T>::Queue(const unsigned int depth)
{
	setQueueDepth(depth);
}

template <class T>
Queue<T>::~Queue()
{
	cout << "Call Queue::~Queue()." << endl;
	if(NULL !=dataArray)
	{
		free(dataArray);
		dataArray = NULL;
	}
	pHead = NULL;
	pTail = NULL;
	queueMaxDepth = 0;
	curDepth = 0;
	cout << "Call Queue::~Queue() end." << endl;
}

template <class T>
int Queue<T>::setQueueDepth(unsigned int depth)
{
	//cout << "sizeof(T) = " << sizeof(T) << endl;	
	//dataArray = new T(depth);

	if(NULL != dataArray)		// 说明已经初始化了dataArray.
	{
		return -1;
	}
	
	dataArray = (T *)malloc(sizeof(T) * depth);
	if(NULL == dataArray)
	{
		pHead = NULL;
		pTail = NULL;
		queueMaxDepth = 0;
		curDepth = 0;
		return -1;
	}

	pHead = dataArray;
	pTail = dataArray;
	queueMaxDepth = depth;
	curDepth = 0;
	
	return 0;
}

template <class T>
int Queue<T>::push(const T *pData)
{
	if(!checkOK())
	{
		return -1;
	}

	bool bRelinQuish = false;
	if(isFull())
	{
		relinquish();
		bRelinQuish = true;
	}

	memcpy(pTail, pData, sizeof(T));
	tailIncrease();
	++curDepth;

	if(bRelinQuish)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

template <class T>
int Queue<T>::pop(T *pData)
{
	if(!checkOK())
	{
		return -1;
	}

	if(isEmpty())
	{
		return -2;
	}

	memcpy(pData, pHead, sizeof(T));
	memset(pHead, 0, sizeof(T));
	headIncrease();
	--curDepth;

	return 0;
}

template <class T>
int Queue<T>::clear()
{
	if(!checkOK())
	{
		return -1;
	}

	while(!isEmpty())
	{
		relinquish();
	}

	memset(dataArray, 0, sizeof(T) * queueMaxDepth);

	return 0;
}

template <class T>
int Queue<T>::relinquish()
{
	//cout << "relinquish()" << endl;		// debug
	if(!checkOK())
	{
		return -1;
	}
	
	if(isEmpty())
	{
		return -2;
	}

	--curDepth;
	memset(pHead, 0, sizeof(T));
	headIncrease();
}

template <class T>
bool Queue<T>::isEmpty()
{
	if(0 == curDepth)
	{
		return true;
	}

	return false;
}

template <class T>
bool Queue<T>::isFull()
{
	if(curDepth == queueMaxDepth)
	{
		return true;
	}

	return false;
}


template <class T>
unsigned int Queue<T>::depth()
{
	return curDepth;
}

template <class T>
unsigned int Queue<T>::maxDepth()
{
	return queueMaxDepth;
}

template <class T>
bool Queue<T>::checkOK()
{
	if(NULL != dataArray && NULL != pHead && NULL != pTail && (curDepth <= queueMaxDepth))
	{
		return true;
	}

	return false;
}

template <class T>
void Queue<T>::headIncrease()
{
	if(!checkOK())
	{
		return;
	}

	++pHead;
	if(pHead == dataArray + queueMaxDepth)
	{
		pHead = dataArray;
	}
}

template <class T>
void Queue<T>::tailIncrease()
{
	if(!checkOK())
	{
		return;
	}

	++pTail;
	if(pTail == dataArray + queueMaxDepth)
	{
		pTail = dataArray;
	}
}

template <class T>
void Queue<T>::output()
{
	if(!checkOK())
	{
		return;
	}

	cout << "output:";
	int i = 0;
	for(i = 0; i < queueMaxDepth; ++i)
	{
		cout << dataArray[i] << ", ";
	}
	cout << endl;
}

#endif
