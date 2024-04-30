#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class JitterBuffer
{
public:
	JitterBuffer() {};
	JitterBuffer(int capacity) : capacity_(capacity)
	{

	}

	void Push(const T item,int expand_size = 0)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		if (expand_size != 0 && expand_size != capacity_)
		{
			capacity_ = expand_size;
		}
		not_full_.wait(lock, [this] {
			  return buffer_.size() < capacity_;
			}
		);
		buffer_.push(item);
		not_empty_.notify_one();
	}

	int size() {
		std::unique_lock<std::mutex> lock(mutex_);
		return  buffer_.size();
	}

	void Clear(std::function<void(T)> method)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		do {
			if (buffer_.empty())
			{
				break;
			}
			T item = buffer_.front();
			if (item == nullptr)
			{
				break;
			}
			buffer_.pop();
			method(item);
		} while (true);

		not_full_.notify_one();
	}


	T Pop(bool isNonblock)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		if (isNonblock && buffer_.empty())
		{
			return nullptr;
		}
		if (!isNonblock)
		{
			not_empty_.wait(lock, [this] {
				return !buffer_.empty();
				}
			);
		}
		
		T item = buffer_.front();
		buffer_.pop();
		not_full_.notify_one();

		return item;
	}
	~JitterBuffer() {
		
	}

private:
	int capacity_;
	std::queue<T> buffer_;
	std::mutex mutex_;
	std::condition_variable not_full_;
	std::condition_variable not_empty_;
};

