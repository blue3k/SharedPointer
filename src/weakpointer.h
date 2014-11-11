

#include "synchronize.h"
#include "sharedpointer.h"

namespace BR
{

	class WeakPointer
	{
	private:

		mutable SharedObject *m_pObject;

	public:

		WeakPointer()
			:m_pObject(nullptr)
		{
		}

		WeakPointer(SharedObject* pRef)
			:m_pObject(pRef)
		{
			if (m_pObject != nullptr)
				Interlocked::Increment(m_pObject->m_WeakReferenceCount);
		}

		WeakPointer(const WeakPointer& src)
			:m_pObject(src.m_pObject)
		{
			if (m_pObject != nullptr)
				Interlocked::Increment(m_pObject->m_WeakReferenceCount);
		}

		~WeakPointer()
		{
			ReleaseReference();
		}

		void ReleaseReference() const
		{
			if (m_pObject == nullptr)
				return;

			auto decValue = Interlocked::Decrement(m_pObject->m_ReferenceCount);
			if (decValue == 0)
			{
				if (m_pObject->m_ReferenceManagerObject != nullptr)
					m_pObject->m_ReferenceManagerObject->FreeWeakReference(m_pObject);
				else
					delete m_pObject;
			}
			m_pObject = nullptr;
		}

		operator SharedPointer()
		{
			return SharedPointer(m_pObject);
		}

		operator const SharedPointer() const
		{
			return SharedPointer(m_pObject);
		}

		WeakPointer& operator = (const WeakPointer& src)
		{
			ReleaseReference();

			if (src.m_pObject == nullptr || src.m_pObject->GetReferenceCount() == 0)
				return *this;

			m_pObject = src.m_pObject;

			if (m_pObject != nullptr)
				Interlocked::Increment(m_pObject->m_ReferenceCount);

			return *this;
		}

	};

}

