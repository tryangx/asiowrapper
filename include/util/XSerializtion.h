/**
*	序列化
*
*
*/
#pragma once

#pragma warning( disable:4244 )
#include <boost/archive/shared_ptr_helper.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace XGAME
{
	//----------------------------

	class XAsioPacketArchive
	{
	public:
		template<class Archive>
		void serialize( Archive& archive, const unsigned int version ) = 0;
	};

	class XAsioPacket : public XAsioPacketArchive
	{
	public:
		template<class Archive>
		void serialize( Archive& archive, const unsigned int version )
		{
			archive & s;
			archive & value;
		}

	public:
		std::string		s;
		int				value;
	};

	class XSerialization
	{

	};

/*
	class naked_binary_iarchive : public binary_iarchive_impl< boost::archive::naked_binary_iarchive, std::istream::char_type, std::istream::traits_type >  
	{  
	public:  
		naked_binary_iarchive(std::istream & is, unsigned int flags = 0) : binary_iarchive_impl<naked_binary_iarchive, std::istream::char_type, std::istream::traits_type>(is, flags) {}  
		naked_binary_iarchive(std::streambuf & bsb, unsigned int flags = 0) :  
		binary_iarchive_impl<  
			naked_binary_iarchive, std::istream::char_type, std::istream::traits_type  
		>(bsb, flags)  
		{}  
	};  

	class msg_binary_iarchive :   
		public binary_iarchive_impl<  
		boost::archive::msg_binary_iarchive,   
		std::istream::char_type,   
		std::istream::traits_type  
		>,  
		public detail::shared_ptr_helper  
	{  
	public:  
		typedef binary_iarchive_impl<  
			boost::archive::msg_binary_iarchive,   
			std::istream::char_type,   
			std::istream::traits_type  
		> base;  
		msg_binary_iarchive(std::istream & is, unsigned int flags = 0) :  
		binary_iarchive_impl<  
			msg_binary_iarchive, std::istream::char_type, std::istream::traits_type  
		>(is, flags)  
		{}  
		msg_binary_iarchive(std::streambuf & bsb, unsigned int flags = 0) :  
		binary_iarchive_impl<  
			msg_binary_iarchive, std::istream::char_type, std::istream::traits_type  
		>(bsb, flags)  
		{}  

		template<class T>  
		void load_override(T & t, BOOST_PFTO int)  
		{  
			BOOST_MPL_ASSERT_NOT(( boost::is_pointer<T> ));  
			base::load_override(t, 0);  
		}  

		// 这些信息都不要了  
		void load_override(boost::archive::class_id_optional_type &, int){}  
		void load_override(boost::archive::tracking_type & t, int){t.t = false;}  
		void load_override(boost::archive::version_type & t, int){t.t = 0;}  
	};  

	// required by export  
	BOOST_SERIALIZATION_REGISTER_ARCHIVE(boost::archive::msg_binary_iarchive)  
		BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(boost::archive::msg_binary_iarchive)  

	class msg_binary_oarchive :   
		public binary_oarchive_impl<  
		msg_binary_oarchive, std::ostream::char_type, std::ostream::traits_type  
		>  
	{  
	public:  
		typedef binary_oarchive_impl<  
			msg_binary_oarchive, std::ostream::char_type, std::ostream::traits_type  
		> base;  
		msg_binary_oarchive(std::ostream & os, unsigned int flags = 0) :  
		binary_oarchive_impl<  
			msg_binary_oarchive, std::ostream::char_type, std::ostream::traits_type  
		>(os, flags)  
		{}  
		msg_binary_oarchive(std::streambuf & bsb, unsigned int flags = 0) :  
		binary_oarchive_impl<  
			msg_binary_oarchive, std::ostream::char_type, std::ostream::traits_type  
		>(bsb, flags)  
		{}  

		template<class T>  
		void save_override(T & t, BOOST_PFTO int)  
		{  
			BOOST_MPL_ASSERT_NOT(( boost::is_pointer<T> ));  
			base::save_override(t, 0);  
		}  

		// 这些信息都不要了  
		void save_override(const boost::archive::class_id_optional_type &, int){}  
		void save_override(const boost::archive::tracking_type &, int){}  
		void save_override(const boost::archive::version_type &, int){}  

	};  

	typedef msg_binary_oarchive naked_binary_oarchive;  

	// required by export  
	BOOST_SERIALIZATION_REGISTER_ARCHIVE(boost::archive::msg_binary_oarchive)  
		BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(boost::archive::msg_binary_oarchive)  

#define MSG1(mn,t1,n1)/  
	struct mn/  
	{/  
	t1 vn1;/  
	template<class Archive> void serialize(Archive & ar, const unsigned int version)/  
	{/  
	ar & vn1;/  
	}/  
	};  

#define MSG2(mn,t1,n1,t2,n2)/  
	struct mn/  
	{/  
	t1 n1;/  
	t2 n2;/  
	template<class Archive> void serialize(Archive & ar, const unsigned int version)/  
	{/  
	ar & n1;/  
	ar & n2;/  
	}/  
	};  

#define MSG3(mn,t1,n1,t2,n2,t3,n3)/  
	struct mn/  
	{/  
	t1 n1;/  
	t2 n2;/  
	t3 n3;/  
	template<class Archive> void serialize(Archive & ar, const unsigned int version)/  
	{/  
	ar & n1;/  
	ar & n2;/  
	ar & n3;/  
	}/  
	};  

#define MSG4(mn,t1,n1,t2,n2,t3,n3,t4,n4)/  
	struct mn/  
	{/  
	t1 n1;/  
	t2 n2;/  
	t3 n3;/  
	t4 n4;/  
	template<class Archive> void serialize(Archive & ar, const unsigned int version)/  
	{/  
	ar & n1;/  
	ar & n2;/  
	ar & n3;/  
	ar & n4;/  
	}/  
	};  

#define MSG5(mn,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5)/  
	struct mn/  
	{/  
	t1 n1;/  
	t2 n2;/  
	t3 n3;/  
	t4 n4;/  
	t5 n5;/  
	template<class Archive> void serialize(Archive & ar, const unsigned int version)/  
	{/  
	ar & n1;/  
	ar & n2;/  
	ar & n3;/  
	ar & n4;/  
	ar & n5;/  
	}/  
	};  

#define MSG6(mn,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6)/  
	struct mn/  
	{/  
	t1 n1;/  
	t2 n2;/  
	t3 n3;/  
	t4 n4;/  
	t5 n5;/  
	t6 n6;/  
	template<class Archive> void serialize(Archive & ar, const unsigned int version)/  
	{/  
	ar & n1;/  
	ar & n2;/  
	ar & n3;/  
	ar & n4;/  
	ar & n5;/  
	ar & n6;/  
	}/  
	};  

#define MSG7(mn,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7)/  
	struct mn/  
	{/  
	t1 n1;/  
	t2 n2;/  
	t3 n3;/  
	t4 n4;/  
	t5 n5;/  
	t6 n6;/  
	t7 n7;/  
	template<class Archive> void serialize(Archive & ar, const unsigned int version)/  
	{/  
	ar & n1;/  
	ar & n2;/  
	ar & n3;/  
	ar & n4;/  
	ar & n5;/  
	ar & n6;/  
	ar & n7;/  
	}/  
	};  

#define MSG8(mn,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7,t8,n8)/  
	struct mn/  
	{/  
	t1 n1;/  
	t2 n2;/  
	t3 n3;/  
	t4 n4;/  
	t5 n5;/  
	t6 n6;/  
	t7 n7;/  
	t8 n8;/  
	template<class Archive> void serialize(Archive & ar, const unsigned int version)/  
	{/  
	ar & n1;/  
	ar & n2;/  
	ar & n3;/  
	ar & n4;/  
	ar & n5;/  
	ar & n6;/  
	ar & n7;/  
	ar & n8;/  
	}/  
	};  

#define MSG9(mn,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7,t8,n8,t9,n9)/  
	struct mn/  
	{/  
	t1 n1;/  
	t2 n2;/  
	t3 n3;/  
	t4 n4;/  
	t5 n5;/  
	t6 n6;/  
	t7 n7;/  
	t8 n8;/  
	t9 n9;/  
	template<class Archive> void serialize(Archive & ar, const unsigned int version)/  
	{/  
	ar & n1;/  
	ar & n2;/  
	ar & n3;/  
	ar & n4;/  
	ar & n5;/  
	ar & n6;/  
	ar & n7;/  
	ar & n8;/  
	ar & n9;/  
	}/  
	};  

	class MsgPack  
	{  
	public:  
		MsgPack():  
		  _oa(_os, boost::archive::no_header)  
		  {}  

		  template <class T>  
		  MsgPack& operator & (const T & v)  
		  {  
			  reset();  
			  _oa & v;  

			  return *this;  
		  }  

		  template <class T>  
		  MsgPack& operator << (const T & v)  
		  {  
			  _oa & v;  

			  return *this;  
		  }  

		  void reset()  
		  {  
			  _os.freeze(false);  
			  _os.seekp(0);  
			  _os.seekg(0);  
		  }  

		  const char* buffer()  
		  {  
			  return _os.str();  
		  }  

		  size_t size()  
		  {  
			  return _os.pcount();  
		  }  

	private:  
		std::strstream                          _os;  
		boost::archive::msg_binary_oarchive     _oa;  
	};  

	class MsgUnpack  
	{  
	public:  
		MsgUnpack():  
		  _ia(_is, boost::archive::no_header)  
		  {}  

		  void reset(const char* buf, size_t size)  
		  {  
			  if (_is.pcount())  
			  {  
				  _is.seekp(0);  
				  _is.seekg(0);  
			  }  
			  _is.write(buf, (std::streamsize)size);  
		  }  

		  template <class T>  
		  MsgUnpack& operator >> (T & v)  
		  {  
			  _ia & v;  

			  return *this;  
		  }  

	private:  
		std::strstream                          _is;  
		boost::archive::msg_binary_iarchive     _ia;  
	};  */
}