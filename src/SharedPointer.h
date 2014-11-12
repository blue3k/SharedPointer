

#include "Thread/synchronize.h"
#include "SharedReferenceManager.h"

namespace BR
{

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Shared pointer reference class
	//

	class SharedPointer
	{
	protected:
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

	template<class ClassType>
	class SharedPointerT : public SharedPointer
	{
	public:
		SharedPointerT()
			:SharedPointer()
		{
		}

		SharedPointerT(ClassType* pRef)
			:SharedPointer(pRef)
		{
		}

		operator ClassType*()
		{
			return (ClassType*)m_pObject;
		}

		operator const ClassType*() const
		{
			return (ClassType*)m_pObject;
		}
	};



	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Weak pointer reference class
	//

	class WeakPointer
	{
	protected:

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

	template<class ClassType>
	class WeakPointerT : public WeakPointer
	{
	public:
		WeakPointerT()
			:WeakPointer()
		{
		}

		WeakPointerT(const WeakPointerT<typename ClassType>& src)
			:WeakPointer(src)
		{
		}


		operator SharedPointerT<ClassType>()
		{
			return SharedPointerT(m_pObject);
		}

		operator const SharedPointerT<ClassType>() const
		{
			return SharedPointerT<ClassType>(m_pObject);
		}

	};

}

