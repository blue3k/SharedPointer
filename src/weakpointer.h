

#include "synchronize.h"
#include "sharedpointer.h"

namespace BR
{

	class WeakPointer
	{
		ObjectRef *m_pObject;
	public:

		WeakPointer()
			:m_pObject(nullptr)
		{
		}

		WeakPointer(ObjectRef* pRef)
			:m_pObject(pRef)
		{
			if (m_pObject != nullptr)
				Interlocked::Increment(m_pObject->WeakReferenceCount);
		}

		~WeakPointer()
		{
			if (m_pObject != nullptr)
			{
				auto decValue = Interlocked::Decrement(m_pObject->WeakReferenceCount);
				if (decValue == 0)
				{
					SharedPointerManager::FreeWeakReference(m_pObject);
				}
				m_pObject = nullptr;
			}
		}

	};

}

