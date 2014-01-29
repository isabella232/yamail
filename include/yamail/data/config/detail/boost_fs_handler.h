#ifndef _YAMAIL_DATA_CONFIG_DETAIL_BOOST_FS_HANDLER_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_BOOST_FS_HANDLER_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/compat/shared_ptr.h>
#include <yamail/data/config/detail/fs_handler.h>

#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include <iterator>
#include <type_traits>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

namespace fs = boost::filesystem;
namespace io = boost::iostreams;

template <typename CharT = char>
class boost_fs_handler
  : public boost::iterator_range<CharT const*>
{
  typedef boost::iterator_range<CharT const*> base_t;

  compat::shared_ptr<io::mapped_file_source> file_;

public:
  typedef return_iterator_range category;

  template <typename Source>
  inline boost_fs_handler (Source const& path)
  {
    open (path);
  }

  template <typename Source>
  void open (Source const& path)
  {
    // just in case
    // fs::path normalized_path = normalize_path_impl (path);

    std::string npath_string = fs::path (path).template string<std::string> ();

    compat::shared_ptr<io::mapped_file_source> file (
        compat::make_shared<io::mapped_file_source> (npath_string));

    this->base_t::operator= ( 
      boost::make_iterator_range (file->data (), file->data () + file->size ())
    );
    
    file_ = std::move (file);
  }

  inline void close ()
  {
    file_.reset ();
  }

  template <typename C, typename T, typename A>
  static inline 
  std::basic_string<C,T,A>
  normalize_path (std::basic_string<C,T,A> const& path)
  {
    return 
      normalize_path_impl (path).template string<std::basic_string<C,T,A> > ();
  }

  template <typename C, std::size_t N>
  static inline 
  std::basic_string<typename std::remove_const<C>::type>
  normalize_path (C (&path)[N])
  {
    typedef typename std::remove_const<C>::type char_type;

    return normalize_path_impl (path).
      template string<std::basic_string<char_type> > ();
  }

  template <typename Iterator>
  static inline
  std::basic_string<
    typename std::decay<
      typename std::iterator_traits<Iterator>::value_type
    >::type
  >
  normalize_path (Iterator path)
  {
    typedef typename std::decay<
      typename std::iterator_traits<Iterator>::value_type
    >::type char_type;

    return normalize_path_impl (path).
      template string<std::basic_string<char_type> > ();
  }

  template <typename Dest, typename Src>
  static inline Dest normalize_path_raw (Src const& path)
  {
    return normalize_path_impl (path). template string<Dest> ();
  }

protected:

  template <typename Source>
  static fs::path normalize_path_impl (Source const& path)
  {
    fs::path abs_path = absolute (fs::path (path));
    fs::path::iterator it = abs_path.begin ();
    fs::path result = *it++;

    // Get canonical version of the existing part
    for (; exists (result) && it != abs_path.end (); ++it)
      result /= *it;

    result = canonical (result.parent_path ());

    // For the rest remove ".." and "." in a path with no symlinks
    for (--it; it != abs_path.end (); ++it)
    {
      // Move back on "..".
      if (*it == "..")
        result = result.parent_path ();

      // Ignore ".".
      else if (*it != ".")
        result /= *it;
    }

    return result;
  }
};

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_BOOST_FS_HANDLER_H_
