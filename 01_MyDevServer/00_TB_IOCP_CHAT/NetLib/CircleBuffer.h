#pragma once
#include <mutex>

class CircleBuffer
{
public:
	CircleBuffer(int size)
	{
		mTotalSize = size;
		mBuffer = new char[size];

		mReadPos = 0;
		mWritePos = 0;
	}
	~CircleBuffer()
	{
		delete[] mBuffer;
		mBuffer = nullptr;
	}

	void Init()
	{
		// 소멸자가 불린게 아님 그냥 깔끔하게 memset만 해주자
		std::lock_guard<std::mutex> guard(mLock);
		memset(mBuffer, 0, mTotalSize);
		mReadPos = 0;
		mWritePos = 0;
	}

	int SetWriteBuffer(char* pData, int size)
	{
		std::lock_guard<std::mutex> guard(mLock);
		
		// 넘으면 그냥 처음부터 쓰자
		if (size > GetWriteAbleSize())
		{
			memcpy_s(&mBuffer[0], size, pData, size);
			mReadPos = 0;
			mWritePos = size;
		}
		else
		{
			memcpy_s(&mBuffer[mWritePos],size, pData, size);
			mWritePos += size;
		}
		return size;
	}

	// 연속으로 쓸 수 있는 사이즈
	int GetWriteAbleSize()
	{
		if (mReadPos > mWritePos)
		{
			return mReadPos - mWritePos;
		}
		else
		{
			return mTotalSize - mWritePos;
		}
	}

	// 연속으로 읽을 수 있는 사이즈
	int	GetReadAbleSize(void)
	{
		if (mReadPos <= mWritePos)
		{
			return mWritePos - mReadPos;
		}

		else
			return (mTotalSize - mReadPos) + mWritePos;
	}


	// WSASend 전에 세팅해줄 함수
	int	GetReadAbleSize2(int size)
	{
		std::lock_guard<std::mutex> guard(mLock);

		// 읽어야될 사이즈가 넘치면 처음부터 다시 읽게 만든다.
		if (size > (mTotalSize - mReadPos))
		{
			mReadPos = 0;
			return mTotalSize;
		}
		else
		{
			return (mTotalSize - mReadPos) + mWritePos;
		}
	}

	// 패킷 토탈 길이 뽑아옴
	int	GetHeaderSize(char* pData, int size)
	{
		std::lock_guard<std::mutex> guard(mLock);
		
		// 이건 잘못된 거다
		if (mTotalSize - mReadPos < size)
		{
			return -1;
		}

		memcpy_s(pData, size, &mBuffer[mReadPos], size);
		return size;
	}



	// Send finish 할때 
	void MoveReadPos(int size)
	{
		std::lock_guard<std::mutex> guard(mLock);
		mReadPos = (mReadPos + size) % mTotalSize;
	}

	// OnRecv에서 패킷 분해하기 전에 호출
	void MoveWritePos(int size)
	{
		std::lock_guard<std::mutex> guard(mLock);
		mWritePos = (mWritePos + size) % mTotalSize;
	}

	int	GetRemainSize()
	{
		if (mReadPos > mWritePos)
			return mReadPos - mWritePos;
		else
			return (mTotalSize - mWritePos) + mReadPos;
	}


	char* GetWriteBufferPtr(){return &mBuffer[mWritePos];}
	int   GetWritePos() { return mWritePos; }
	char* GetBufferPtr(void) { return mBuffer; }
	char* GetReadBufferPtr(void) { return &mBuffer[mReadPos]; }
private:
	char* mBuffer;
	int mTotalSize = 0;
	int mReadPos = 0;
	int mWritePos = 0;
	std::mutex	mLock;

};