#ifndef CXC_ANIMCOMPONENT_H
#define CXC_ANIMCOMPONENT_H

#include "Components/CComponent.h"
#include <memory>

namespace cxc
{
	class AnimContext;

	class CAnimComponent : public CComponent
	{
	public:
		CAnimComponent();
		virtual ~CAnimComponent();

	public:

		void CreateAnimContext();
		std::shared_ptr<AnimContext> GetAnimContext() { return pAnimContext; }

	protected:

		std::shared_ptr<AnimContext> pAnimContext;
	};
}

#endif // CXC_ANIMCOMPONENT_H