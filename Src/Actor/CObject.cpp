#include "Actor/CObject.h"
#include "Components/CComponent.h"

namespace cxc
{
	CObject::CObject() :
		bIsEnabled(true)
	{

	}

	CObject::CObject(const std::string& NewName, const std::string& NewTag)
	{
		Name = NewName; 
		bIsEnabled = true;
		Tag = NewTag;
	}

	CObject::~CObject()
	{

	}

	void CObject::SetRootComponent(std::shared_ptr<CComponent> Root)
	{
		if (Root)
		{
			RootComponent = Root;
			AttachedComponents.push_back(Root);
		}
	}

	void CObject::AddAttachedComponent(std::shared_ptr<CComponent> Component)
	{
		if(Component)
			AttachedComponents.push_back(Component);
	}

	void CObject::Tick(float DeltaSeconds)
	{
		// Tick all the components
		for (auto Component : AttachedComponents)
		{
			if (Component)
				Component->Tick(DeltaSeconds);
		}
	}
}