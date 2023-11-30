#pragma once

namespace HOL
{
	struct FingerBend
	{
		float curl[3];
		float splay;

		float getCurlSum();
	};
} // namespace HOL
