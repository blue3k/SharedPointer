////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2014 MadK
// 
// Author : KyungKun Ko
//
// Description : Shared pointer
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Thread/Synchronize.h"
#include "SharedObject.h"

namespace BR
{
	class WeakPointer;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Shared pointer reference class
	//

	class SharedPointer
	{
	public:
		static SharedPointer NullValue;

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
			m_pObject->AddReference();
		}

		SharedPointer(SharedObject* pRef)
			:m_pObject(pRef)
		{
			if (m_pObject != nullptr)
			{
				m_pObject->AddReference();
			}
		}

		~SharedPointer()
		{
			ReleaseReference();
		}

		void ReleaseReference() const
		{
			if (m_pObject == nullptr)
				return;

			//assert(m_pObject->m_ReferenceCount > 0);

			m_pObject->ReleaseReference();

			m_pObject = nullptr;
		}

		explicit operator SharedObject*()
		{
			return m_pObject;
		}

		explicit operator const SharedObject*() const
		{
			return m_pObject;
		}

		SharedPointer& operator = (const SharedPointer& src)
		{
			ReleaseReference();

			m_pObject = src.m_pObject;

			if (m_pObject != nullptr)
			{
				Assert(m_pObject->GetWeakReferenceCount() > 0 || m_pObject->GetReferenceCount() > 0);
				m_pObject->AddReference();
			}

			return *this;
		}


		bool operator == (const SharedPointer& src) const
		{
			return m_pObject == src.m_pObject;
		}

		bool operator != (const SharedPointer& src) const
		{
			return m_pObject != src.m_pObject;
		}

		bool operator == (SharedObject* pRef) const
		{
			return m_pObject == pRef;
		}

		bool operator != (SharedObject* pRef) const
		{
			return m_pObject != pRef;
		}

	protected:

		virtual void SetPointer(SharedObject* pObject)
		{
			ReleaseReference();

			m_pObject = pObject;
		}

		friend class SharedObject;
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

		explicit operator ClassType*()
		{
			return (ClassType*)m_pObject;
		}

		explicit operator const ClassType*() const
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

		bool operator == (const SharedPointer& src) const
		{
			return __super::operator == (src);
		}

		bool operator != (const SharedPointer& src) const
		{
			return __super::operator != (src);
		}

		bool operator == (SharedObject* pRef) const
		{
			return __super::operator == (pRef);
		}

		bool operator != (SharedObject* pRef) const
		{
			return __super::operator != (pRef);
		}

		SharedPointerT<ClassType>& operator = (const SharedPointer& src)
		{
			auto pObjectSrc = (SharedObject*)(const SharedObject*)src;
			if (pObjectSrc != nullptr)
			{
				if (!TypeCheck(pObject))
					return *this;
			}

			__super::operator = (src);

			return *this;
		}

		SharedPointerT<ClassType>& operator = (const SharedPointerT<ClassType>& src)
		{
			__super::operator = (src);
			return *this;
		}


	protected:

		bool TypeCheck(SharedObject* pObject)
		{
			if (pObject == nullptr) return true;

			auto type = typeid(*pObject).name();
			auto type2 = typeid(ClassType).name();
			bool sameType = type == type2;
			assert(sameType);
			return sameType;
		}

		virtual void SetPointer(SharedObject* pObject) override
		{
			if (!TypeCheck(pObject))
				return;

			__super::SetPointer(pObject);
		}
	};



	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Weak pointer reference class
	//

	class WeakPointer
	{
	public:
		static WeakPointer NullValue;

	protected:

		mutable SharedObject *m_pObject;

		WeakPointer(SharedObject* pRef)
			:m_pObject(pRef)
		{
			if (m_pObject != nullptr)
				m_pObject->AddWeakReference();
		}

	public:

		WeakPointer()
			:m_pObject(nullptr)
		{
		}

		WeakPointer(const WeakPointer& src)
			:m_pObject(src.m_pObject)
		{
			m_pObject->AddWeakReference();
		}

		~WeakPointer()
		{
			ReleaseReference();
		}

		void ReleaseReference() const
		{
			if (m_pObject == nullptr)
				return;

			m_pObject->ReleaseWeakReference();

			m_pObject = nullptr;
		}

		template<class SharedPointerType>
		void GetSharedPointer(SharedPointerType& pointer) const
		{
			if (m_pObject != nullptr)
				m_pObject->GetSharedPointer(pointer);
			else
				pointer = SharedPointerType();
		}

		explicit operator SharedPointer()
		{
			SharedPointer pointer;
			GetSharedPointer(pointer);
			return pointer;
		}

		explicit operator const SharedPointer() const
		{
			SharedPointer pointer;
			GetSharedPointer(pointer);
			return pointer;
		}

		bool operator == (const SharedPointer& src) const
		{
			return m_pObject == (const SharedObject*)src;
		}

		bool operator != (const SharedPointer& src) const
		{
			return m_pObject != (const SharedObject*)src;
		}

		bool operator == (const WeakPointer& src) const
		{
			return m_pObject == src.m_pObject;
		}

		bool operator != (const WeakPointer& src) const
		{
			return m_pObject != src.m_pObject;
		}

		bool operator == (SharedObject* pRef) const
		{
			return m_pObject == pRef;
		}

		bool operator != (SharedObject* pRef) const
		{
			return m_pObject != pRef;
		}

		WeakPointer& operator = (const SharedPointer& src)
		{
			ReleaseReference();

			auto *pObj = const_cast<SharedObject*>((const SharedObject*)src);
			if (pObj == nullptr)
				return *this;

			m_pObject = pObj;

			if (m_pObject != nullptr)
			{
				//Assert(m_pObject->GetWeakReferenceCount() > 0 || m_pObject->GetReferenceCount() > 0);
				m_pObject->AddWeakReference();
			}

			return *this;
		}

		WeakPointer& operator = (const WeakPointer& src)
		{
			ReleaseReference();

			if (src.m_pObject == nullptr || src.m_pObject->GetReferenceCount() == 0)
				return *this;

			m_pObject = src.m_pObject;

			if (m_pObject != nullptr)
			{
				//AssertRel(m_pObject->GetWeakReferenceCount() > 0 || m_pObject->GetReferenceCount() > 0);
				m_pObject->AddWeakReference();
			}

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

		explicit operator SharedPointerT<ClassType>()
		{
			SharedPointerT<ClassType> pointer;
			GetSharedPointer(pointer);
			return pointer;
		}

		explicit operator const SharedPointerT<ClassType>() const
		{
			SharedPointerT<ClassType> pointer;
			GetSharedPointer(pointer);
			return pointer;
		}

		bool operator == (const SharedPointer& src) const
		{
			return __super::operator == (src);
		}

		bool operator != (const SharedPointer& src) const
		{
			return __super::operator != (src);
		}

		bool operator == (const WeakPointer& src) const
		{
			return __super::operator == (src);
		}

		bool operator != (const WeakPointer& src) const
		{
			return __super::operator != (src);
		}

		bool operator == (SharedObject* pRef) const
		{
			return __super::operator == (pRef);
		}

		bool operator != (SharedObject* pRef) const
		{
			return __super::operator != (pRef);
		}

		WeakPointerT<ClassType>& operator = (const SharedPointerT<ClassType>& src)
		{
			__super::operator = (src);

			return *this;
		}

		WeakPointerT<ClassType>& operator = (const SharedPointer& src)
		{
			__super::operator = (src);

			if (m_pObject != nullptr)
			{
				assert(typeid(m_pObject) == typeid(ClassType));
			}

			return *this;
		}

		WeakPointerT<ClassType>& operator = (const WeakPointer& src)
		{
			__super::operator = (src);

			if (m_pObject != nullptr)
			{
				assert(typeid(m_pObject) == typeid(ClassType));
			}

			return *this;
		}

		WeakPointerT<ClassType>& operator = (const WeakPointerT<ClassType>& src)
		{
			__super::operator = (src);

			return *this;
		}

	};

}

