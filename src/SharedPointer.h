

#include "Thread/synchronize.h"
#include "SharedReferenceManager.h"

namespace BR
{
	class WeakPointer;

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

		SharedPointer(const SharedPointer& src)
			:m_pObject(src.m_pObject)
		{
			if (m_pObject != nullptr)
				Interlocked::Increment(m_pObject->m_ReferenceCount);
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

			assert(m_pObject->m_ReferenceCount > 0);

			Interlocked::Increment(m_pObject->m_ManageCount);

			auto decValue = Interlocked::Decrement(m_pObject->m_ReferenceCount);
			if (decValue <= 0)
			{
				if (m_pObject->m_ReferenceManagerObject != nullptr)
				{
					m_pObject->m_ReferenceManagerObject->FreeSharedReference(m_pObject);
					Interlocked::Decrement(m_pObject->m_ManageCount);
				}
				else
					delete m_pObject;
			}
			else
			{
				Interlocked::Decrement(m_pObject->m_ManageCount);
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

		WeakPointer GetWeakPointer();

		const WeakPointer GetWeakPointer() const;


		SharedPointer& operator = (const SharedPointer& src)
		{
			if (src.m_pObject != nullptr)
				Interlocked::Increment(src.m_pObject->m_ReferenceCount);

			ReleaseReference();

			m_pObject = src.m_pObject;

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

		SharedPointerT(const SharedPointerT<ClassType>& src)
			:SharedPointer(src)
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

		ClassType* operator ->()
		{
			return (ClassType*)m_pObject;
		}

		ClassType* operator ->() const
		{
			return (ClassType*)m_pObject;
		}

		SharedPointerT<ClassType>& operator = (const SharedPointerT<ClassType>& src)
		{
			__super::operator = (src);
			return *this;
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

			// increase manager reference first
			Interlocked::Increment(m_pObject->m_ManageCount);

			auto decValue = Interlocked::Decrement(m_pObject->m_WeakReferenceCount);
			if (decValue <= 0)
			{
				if (m_pObject->m_ReferenceManagerObject != nullptr)
				{
					m_pObject->m_ReferenceManagerObject->FreeWeakReference(m_pObject);
					Interlocked::Decrement(m_pObject->m_ManageCount);
				}
				else
					delete m_pObject;
			}
			else
			{
				Interlocked::Decrement(m_pObject->m_ManageCount);
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
				Interlocked::Increment(m_pObject->m_WeakReferenceCount);

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

		WeakPointerT(const SharedPointerT<typename ClassType>& src)
			:WeakPointer((ClassType*)(const ClassType*)src)
		{
		}

		WeakPointerT(const WeakPointerT<typename ClassType>& src)
			:WeakPointer(src)
		{
		}

		SharedPointerT<ClassType> ToShared()
		{
			if (m_pObject == nullptr || m_pObject->GetReferenceCount() == 0 || m_pObject->GetIsDisposed())
				return SharedPointerT<ClassType>();

			return SharedPointerT<ClassType>((ClassType*)m_pObject);
		}

		const SharedPointerT<ClassType> ToShared() const 
		{
			if (m_pObject == nullptr || m_pObject->GetReferenceCount() == 0 || m_pObject->GetIsDisposed())
				return SharedPointerT<ClassType>();

			return SharedPointerT<ClassType>((ClassType*)m_pObject);
		}

		operator SharedPointerT<ClassType>()
		{
			return ToShared();
		}

		operator const SharedPointerT<ClassType>() const
		{
			return ToShared();
		}

		WeakPointerT<ClassType>& operator = (const WeakPointerT<ClassType>& src)
		{
			__super::operator = (src);

			return *this;
		}

	};

}

