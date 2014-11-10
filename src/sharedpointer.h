

#include "synchronize.h"
#include "sharedpointermanager.h"

namespace BR
{


	class SharedPointer
	{
		ObjectRef *m_pObject;
	public:

		SharedPointer()
			:m_pObject(nullptr)
		{
		}

		SharedPointer(ObjectRef* pRef)
			:m_pObject(pRef)
		{
			if (m_pObject != nullptr)
				Interlocked::Increment(m_pObject->ReferenceCount);
		}

		~SharedPointer()
		{
			if (m_pObject != nullptr)
			{
				auto decValue = Interlocked::Decrement(m_pObject->ReferenceCount);
				if (decValue == 0)
				{
					SharedPointerManager::FreeSharedReference(m_pObject);
				}
				m_pObject = nullptr;
			}
		}

	};

}

