

#include "synchronize.h"
#include "sharedreferencemanager.h"

namespace BR
{


	class SharedPointer
	{
	private:
		mutable SharedObject *m_pObject;

	public:

		SharedPointer()
			:m_pObject(nullptr)
		{
		}

		SharedPointer(SharedObject* pRef)
			:m_pObject(pRef)
		{
			if (m_pObject != nullptr)
				Interlocked::Increment(m_pObject->m_ReferenceCount);
		}

		~SharedPointer()
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
					m_pObject->m_ReferenceManagerObject->FreeSharedReference(m_pObject);
				else
					delete m_pObject;
			}
			m_pObject = nullptr;
		}

		operator SharedObject*()
		{
			return m_pObject;
		}

		operator const SharedObject*() const
		{
			return m_pObject;
		}


		SharedPointer& operator = (SharedObject* pRef)
		{
			ReleaseReference();

			if (pRef == nullptr || pRef->GetReferenceCount() == 0)
				return *this;

			m_pObject = pRef;

			if (m_pObject != nullptr)
				Interlocked::Increment(m_pObject->m_ReferenceCount);

			return *this;
		}


		bool operator == (SharedObject* pRef) const
		{
			return m_pObject == pRef;
		}

		bool operator != (SharedObject* pRef) const
		{
			return m_pObject != pRef;
		}
	};

}

