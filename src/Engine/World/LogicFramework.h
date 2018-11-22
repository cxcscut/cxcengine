
#ifdef WIN32

#include "..\inl\Singleton.inl"

#else

#include "../inl/Singleton.inl"

#endif

#ifndef CXC_LOGICFRAMEWORK_H
#define CXC_LOGICFRAMEWORK_H

namespace cxc
{
	class LogicFramework final : public Singleton<LogicFramework>
	{
		friend class Singleton<LogicFramework>;

	public:

		LogicFramework();
		~LogicFramework();

	public:

		void SetLogicFrameRates(uint32_t NewFrameRates) { LogicFrameRates = NewFrameRates; }
		uint32_t GetLogicFrameRates() const { return LogicFrameRates; }


	public:

		void LogicTick(float Seconds);

	private:

		// Logic frame rates
		uint32_t LogicFrameRates;
	};
}

#endif // CXC_LOGICFRAMEWORK_H