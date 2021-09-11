#pragma once

template <typename T>
inline void clearVectorOfPtr(std::vector<T>& vector)
{
	for (auto* item : vector)
	{
		delete item;
	}
	vector.clear();
}