#ifndef PCL_OCTREE_DISK_CONTAINER_IMPL_H_
#define PCL_OCTREE_DISK_CONTAINER_IMPL_H_

/*
  Copyright (c) 2012, Urban Robotics Inc
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  * Neither the name of Urban Robotics Inc nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
  This code defines the octree used for point storage at Urban Robotics. Please
  contact Jacob Schloss <jacob.schloss@urbanrobotics.net> with any questions.
  http://www.urbanrobotics.net/
*/

// C++
#include <sstream>
#include <cassert>
#include <ctime>

// Boost
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/bernoulli_distribution.hpp>

// PCL (Urban Robotics)
#include "pcl/outofcore/octree_disk_container.h"

//allows operation on POSIX
#ifndef WIN32
#define _fseeki64 fseeko
#endif

template<typename PointT>
boost::mutex octree_disk_container<PointT>::rng_mutex;

template<typename PointT> boost::mt19937
octree_disk_container<PointT>::rand_gen (std::time( NULL));

template<typename PointT>
boost::uuids::random_generator octree_disk_container<PointT>::uuid_gen (&rand_gen);

template<typename PointT> void
octree_disk_container<PointT>::getRandomUUIDString (std::string& s)
{
  boost::uuids::uuid u;
  {
    boost::mutex::scoped_lock lock (rng_mutex);
    u = uuid_gen ();
  }

  std::stringstream ss;
  ss << u;
  s = ss.str ();
}

template<typename PointT>
octree_disk_container<PointT>::octree_disk_container ()
{
  std::string temp = getRandomUUIDString ();
  fileback_name = new std::string ();
  *fileback_name = temp;
  filelen = 0;
  //writebuff.reserve(writebuffmax);
}

template<typename PointT>
octree_disk_container<PointT>::octree_disk_container (const boost::filesystem::path& path)
{
  if (boost::filesystem::exists (path))
  {
    if (boost::filesystem::is_directory (path))
    {
      std::string uuid;
      getRandomUUIDString (uuid);
      boost::filesystem::path filename (uuid);
      boost::filesystem::path file = path / filename;

      fileback_name = new std::string (file.string ());

      filelen = 0;
    }
    else
    {
      boost::uint64_t len = boost::filesystem::file_size (path);

      fileback_name = new std::string (path.string ());

      filelen = len / sizeof(PointT);
    }
  }
  else
  {
    fileback_name = new std::string (path.string ());
    filelen = 0;
  }

  //writebuff.reserve(writebuffmax);
}

template<typename PointT>
octree_disk_container<PointT>::~octree_disk_container ()
{
  flush_writebuff (true);
  //fileback.flush();
  //fileback.close();
  //std::remove(persistant->c_str());
  //boost::filesystem::remove(fileback_name);//for testing!
  //std::cerr << "deleted file " << *persistant << std::endl;
  //std::cerr << "destruct container" << std::endl;
  delete fileback_name;
}

template<typename PointT> void
octree_disk_container<PointT>::flush_writebuff (const bool forceCacheDeAlloc)
{
  if (writebuff.size () > 0)
  {
    FILE* f = fopen (fileback_name->c_str (), "a+b");

    size_t len = writebuff.size () * sizeof(PointT);
    char* loc = (char*)&(writebuff.front ());
    size_t w = fwrite (loc, 1, len, f);
    assert (w == len);

    //		int closeret = fclose(f);
    fclose (f);

    filelen += writebuff.size ();
    writebuff.clear ();
  }

  //if(forceCacheDeAlloc || (size() >= node<octree_disk_container<PointT>, PointT>::split_thresh))  //if told to dump cache, or if we have more nodes than the split threshold
  if (forceCacheDeAlloc)//if told to dump cache, or if we have more nodes than the split threshold
  {
    //don't reserve anymore -- lets us have more nodes and be fairly
    //lazy about dumping the cache. but once it is dumped for the last
    //time, this needs to be empty.
    writebuff.resize (0);
  }
  else
  {
    //we are still accepting data, preallocate the storage
    //writebuff.reserve(writebuffmax);
  }

}

template<typename PointT> PointT
octree_disk_container<PointT>::operator[] (boost::uint64_t idx)
{
  if (idx < filelen)
  {
    PointT temp;

    FILE* f = fopen (fileback_name->c_str (), "rb");
    assert (f != NULL);
    int seekret = _fseeki64 (f, idx * sizeof(PointT), SEEK_SET);
    assert (seekret == 0);
    size_t readlen = fread (&temp, 1, sizeof(PointT), f);
    assert (readlen == sizeof(PointT));
    int closeret = fclose (f);

    //fileback.open(fileback_name->c_str(), std::fstream::in|std::fstream::out|std::fstream::binary);
    //fileback.seekp(idx*sizeof(PointT), std::ios_base::beg);
    //fileback.read((char*)&temp, sizeof(PointT));
    //fileback.close();
    return temp;
  }

  if (idx < (filelen + writebuff.size ()))
  {
    idx -= filelen;
    return writebuff[idx];
  }
  throw("out of range");
}

template<typename PointT> void
octree_disk_container<PointT>::readRange (const boost::uint64_t start, const boost::uint64_t count,
                                             std::vector<PointT>& v)
{
  if ((start + count) > size ())
  {
    throw("out of range");
  }

  if (count == 0)
  {
    return;
  }

  boost::uint64_t filestart;
  boost::uint64_t filecount;

  boost::int64_t buffstart = -1;
  boost::int64_t buffcount = -1;

  if (start < filelen)
  {
    filestart = start;
  }

  if ((start + count) <= filelen)
  {
    filecount = count;
  }
  else
  {
    filecount = filelen - start;

    buffstart = 0;
    buffcount = count - filecount;
  }

  //resize
  PointT* loc = NULL;
  v.resize ((unsigned int)count);
  loc = &(v.front ());

  //do the read
  FILE* f = fopen (fileback_name->c_str (), "rb");
  assert (f != NULL);
  int seekret = _fseeki64 (f, filestart * static_cast<boost::uint64_t>(sizeof(PointT)), SEEK_SET);
  assert (seekret == 0);

  //read at most 2 million elements at a time
  const static boost::uint32_t blocksize = boost::uint32_t (2e6);
  for (boost::uint64_t pos = 0; pos < filecount; pos += blocksize)
  {
    if ((pos + blocksize) < filecount)
    {
      size_t readlen = fread (loc, sizeof(PointT), blocksize, f);
      assert (readlen == blocksize);
      loc += blocksize;
    }
    else
    {
      size_t readlen = fread (loc, sizeof(PointT), (size_t) (filecount - pos), f);
      assert (readlen == filecount - pos);
      loc += filecount - pos;
    }
  }

  //	int closeret = fclose(f);
  fclose (f);

  //copy the extra
  if (buffstart != -1)
  {
    typename std::vector<PointT>::const_iterator start = writebuff.begin ();
    typename std::vector<PointT>::const_iterator end = writebuff.begin ();

    std::advance (start, buffstart);
    std::advance (end, buffstart + buffcount);

    v.insert (v.end (), start, end);
  }

}

template<typename PointT> void
octree_disk_container<PointT>::readRangeSubSample_bernoulli (const boost::uint64_t start,
                                                                const boost::uint64_t count, 
                                                                const double percent,
                                                                std::vector<PointT>& v)
{
  if (count == 0)
  {
    return;
  }

  v.clear ();

  boost::uint64_t filestart;
  boost::uint64_t filecount;

  boost::int64_t buffstart = -1;
  boost::int64_t buffcount = -1;

  if (start < filelen)
  {
    filestart = start;
  }

  if ((start + count) <= filelen)
  {
    filecount = count;
  }
  else
  {
    filecount = filelen - start;

    buffstart = 0;
    buffcount = count - filecount;
  }

  if (buffcount > 0)
  {
    {
      boost::mutex::scoped_lock lock (rng_mutex);
      boost::bernoulli_distribution<double> buffdist (percent);
      boost::variate_generator<boost::mt19937&, boost::bernoulli_distribution<double> > buffcoin (rand_gen, buffdist);

      for (boost::uint64_t i = buffstart; i < buffcount; i++)
      {
        if (buffcoin ())
        {
          v.push_back (writebuff[i]);
        }
      }
    }
  }

  if (filecount > 0)
  {
    //pregen and then sort the offsets to reduce the amount of seek
    std::vector < boost::uint64_t > offsets;
    {
      boost::mutex::scoped_lock lock (rng_mutex);

      boost::bernoulli_distribution<double> filedist (percent);
      boost::variate_generator<boost::mt19937&, boost::bernoulli_distribution<double> > filecoin (rand_gen, filedist);
      for (boost::uint64_t i = filestart; i < (filestart + filecount); i++)
      {
        if (filecoin ())
        {
          offsets.push_back (i);
        }
      }
    }
    std::sort (offsets.begin (), offsets.end ());

    FILE* f = fopen (fileback_name->c_str (), "rb");
    assert (f != NULL);
    PointT p;
    char* loc = (char*)&p;
    boost::uint64_t filesamp = offsets.size ();
    for (boost::uint64_t i = 0; i < filesamp; i++)
    {
      int seekret = _fseeki64 (f, offsets[i] * static_cast<boost::uint64_t>(sizeof(PointT)), SEEK_SET);
      assert (seekret == 0);
      size_t readlen = fread (loc, sizeof(PointT), 1, f);
      assert (readlen == 1);

      v.push_back (p);
    }
    int closeret = fclose (f);
  }
}

//change this to use a weighted coin flip, to allow sparse sampling of small clouds (eg the bernoulli above)
template<typename PointT> void
octree_disk_container<PointT>::readRangeSubSample (const boost::uint64_t start, const boost::uint64_t count,
                                                      const double percent, std::vector<PointT>& v)
{
  if (count == 0)
  {
    return;
  }

  v.clear ();

  boost::uint64_t filestart;
  boost::uint64_t filecount;

  boost::int64_t buffstart = -1;
  boost::int64_t buffcount = -1;

  if (start < filelen)
  {
    filestart = start;
  }

  if ((start + count) <= filelen)
  {
    filecount = count;
  }
  else
  {
    filecount = filelen - start;

    buffstart = 0;
    buffcount = count - filecount;
  }

  boost::uint64_t filesamp = boost::uint64_t (percent * filecount);
  boost::uint64_t buffsamp = (buffcount > 0) ? (static_cast<boost::uint64_t > (percent * buffcount) ) : 0;

  if ((filesamp == 0) && (buffsamp == 0) && (size () > 0))
  {
    //std::cerr << "would not add points to LOD, falling back to bernoulli";
    readRangeSubSample_bernoulli (start, count, percent, v);
    return;
  }

  if (buffcount > 0)
  {
    {
      boost::mutex::scoped_lock lock (rng_mutex);

      boost::uniform_int < boost::uint64_t > buffdist (0, buffcount - 1);
      boost::variate_generator<boost::mt19937&, boost::uniform_int<boost::uint64_t> > buffdie (rand_gen, buffdist);

      for (boost::uint64_t i = 0; i < buffsamp; i++)
      {
        boost::uint64_t buffstart = buffdie ();
        v.push_back (writebuff[buffstart]);
      }
    }
  }

  if (filesamp > 0)
  {
    //pregen and then sort the offsets to reduce the amount of seek
    std::vector < boost::uint64_t > offsets;
    {
      boost::mutex::scoped_lock lock (rng_mutex);

      offsets.resize (filesamp);
      boost::uniform_int < boost::uint64_t > filedist (filestart, filestart + filecount - 1);
      boost::variate_generator<boost::mt19937&, boost::uniform_int<boost::uint64_t> > filedie (rand_gen, filedist);
      for (boost::uint64_t i = 0; i < filesamp; i++)
      {
        boost::uint64_t filestart = filedie ();
        offsets[i] = filestart;
      }
    }
    std::sort (offsets.begin (), offsets.end ());

    FILE* f = fopen (fileback_name->c_str (), "rb");
    assert (f != NULL);
    PointT p;
    char* loc = (char*)&p;
    for (boost::uint64_t i = 0; i < filesamp; i++)
    {
      int seekret = _fseeki64 (f, offsets[i] * static_cast<boost::uint64_t> (sizeof(PointT)), SEEK_SET);
      assert (seekret == 0);
      size_t readlen = fread (loc, sizeof(PointT), 1, f);
      assert (readlen == 1);

      v.push_back (p);
    }
    int closeret = fclose (f);
  }
}

template<typename PointT> inline void
octree_disk_container<PointT>::push_back (const PointT& p)
{
  writebuff.push_back (p);
  if (writebuff.size () > writebuffmax)
  {
    flush_writebuff (false);
  }
}

template<typename PointT> inline void
octree_disk_container<PointT>::insertRange (const PointT* start, const boost::uint64_t count)
{
  FILE* f = fopen (fileback_name->c_str (), "a+b");

  //write at most 2 million elements at a ime
  const static size_t blocksize = (size_t)2e6;

  for (boost::uint64_t pos = 0; pos < count; pos += blocksize)
  {
    const PointT* loc = start + pos;
    if ((pos + blocksize) < count)
    {
      size_t w = fwrite (loc, sizeof(PointT), blocksize, f);
      assert (w == blocksize);
    }
    else
    {
      size_t w = fwrite (loc, sizeof(PointT), (size_t) (count - pos), f);
      assert (w == count - pos);
    }
  }

  //	int closeret = fclose(f);
  fclose (f);

  filelen += count;
}

#endif //PCL_OCTREE_DISK_CONTAINER_IMPL_H_
