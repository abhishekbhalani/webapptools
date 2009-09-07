/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/thread.hpp>
#include <fstream>
#include <strstream>
#include "storageFS.h"
#include "version.h"
#include "fsStorage.xpm"
//#include "demoPlugin.xrc"
#include <weDispatch.h>

static char* tables[] = {weObjTypeTask,
                        weObjTypeSysOption,
                        weObjTypeDictionary,
                        weObjTypeAuthInfo,
                        weObjTypeScan,
                        weObjTypeScanData,
                        weObjTypeObject,
                        weObjTypeProfile,
                        NULL}; // close the list with NULL

FsStorage::FsStorage( Dispatch* krnl, void* handle /*= NULL*/ ) :
    iStorage(krnl, handle)
{
    pluginInfo.IfaceName = "FsStorage";
    pluginInfo.IfaceList.push_back("FsStorage");
    pluginInfo.PluginDesc = "FileSystem storage";
    pluginInfo.PluginId = "1FBAB8DCF440";
    pluginInfo.PluginIcon = WeXpmToStringList(fsStorage_xpm, sizeof(fsStorage_xpm) / sizeof(char*) );
    db_dir = "";
    lastId = 0;
    LOG4CXX_TRACE(logger, "FsStorage plugin created");
}

FsStorage::~FsStorage(void)
{
    LOG4CXX_TRACE(logger, "FsStorage plugin destroyed");
}

void* FsStorage::GetInterface( const string& ifName )
{
    LOG4CXX_TRACE(logger, "FsStorage::GetInterface " << ifName);
    if (iequals(ifName, "FsStorage"))
    {
        LOG4CXX_DEBUG(logger, "FsStorage::GetInterface found!");
        usageCount++;
        return (void*)(this);
    }
    return iStorage::GetInterface(ifName);
}

const string FsStorage::GetSetupUI( void )
{
    /// @todo: change XRC to set the fields values
    return "";
}

void FsStorage::ApplySettings( const string& xmlData )
{

}

bool FsStorage::InitStorage(const string& params)
{
    bool retval = false;

    LOG4CXX_TRACE(logger, "FsStorage::InitStorage params = " << params);
    try {
        db_dir = params;
        fs::path dir_path(db_dir);
        if ( !fs::exists(dir_path) ) {
            fs::create_directory(db_dir.c_str());
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                dir_path = dir_path.remove_filename();
                db_dir = dir_path.string();
            }
        }
        LOG4CXX_TRACE(logger, "FsStorage::InitStorage: base dir is " << db_dir);

        int i = 0;
        while (tables[i] != NULL)
        {
            // check storage presence
            dir_path = db_dir;
            dir_path /= tables[i];
            if ( !fs::exists(dir_path) ) {
                fs::create_directory(dir_path);
        }
        else {
            if ( ! fs::is_directory(dir_path) ) {
                string msg = dir_path.string() + "isn't a directory";
                    throw std::runtime_error(msg.c_str());
                }
            }
            LOG4CXX_TRACE(logger, "FsStorage::InitStorage: " << tables[i] << " storage dir is " << dir_path.string());
            i++;
        }
    }
    catch(std::exception& e) {
        LOG4CXX_ERROR(logger, "FsStorage::InitStorage: " << e.what());
        retval = false;
    }
    return retval;
}

void FsStorage::Flush(const string& params /*= ""*/)
{
    // nothing to do. Just reimplement base class method
}

string FsStorage::GenerateID(const string& objType /*= ""*/)
{
    string retval = "";
    fs::path dir_path(db_dir);

    dir_path /= objType;

    do {
        retval = lexical_cast<string>(++lastId);
    } while (fs::exists(dir_path / retval));

    return retval;
}

int FsStorage::Get(Record& filter, Record& respFilter, RecordSet& results)
{
    fs::path dir_path(db_dir);
    fs::path locker(db_dir);
    RecordSet* dta;
    StringList reportNames;
    wOption opt;
    size_t i, j;
    Record lk;
    lk.objectID = "locker";

    LOG4CXX_TRACE(logger, "FsStorage::Get objType=" << filter.objectID);
    // lock the db
    while (exists(locker / "lock"))
    {
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
    FileSave(locker, "lock", lk);

    // if no options in the filter - get all records
    dta = Search(filter, (filter.OptionSize() == 0));

    // prepare filter list
    reportNames = respFilter.OptionsList();
    if (reportNames.size() == 0)
    {
        StringList* strct;
        strct = GetStruct(filter.objectID);
        if (strct != NULL)
        {
            reportNames = *strct;
            delete strct;
        }
    }
    for (i = 0; i < dta->size(); i++) {
        Record* objRes = new Record;
        objRes->objectID = filter.objectID;
        for (j = 0; j < reportNames.size(); j++)
        {
            opt = (*dta)[i].Option(reportNames[j]);
            objRes->Option(reportNames[j], opt.Value());
        } // and of attribute filters
        results.push_back(*objRes);
    } // end of objects search

    FileRemove(locker, "lock");
    return results.size();
}

int FsStorage::Set(Record& filter, Record& data)
{
    wOption opt;
    string fName;
    RecordSet* retlist = Search(filter);
    fs::path fp;
    Record sv;
    int retval = 0;

    LOG4CXX_TRACE(iLogger::GetLogger(), "FsStorage::Set(Record, Record)");
    if (retlist->size() > 0)
    {   // update
        LOG4CXX_TRACE(iLogger::GetLogger(), "FsStorage::Set(Record, Record) - update type=" << filter.objectID);
        for (size_t i = 0; i < retlist->size(); i ++)
        {
            sv = (*retlist)[i];
            sv.CopyOptions(&data);
            opt = sv.Option("FsStorage::FileName");
            SAFE_GET_OPTION_VAL(opt, fName, "");
            if (fName == "")
            {
                opt = sv.Option(weoID);
                SAFE_GET_OPTION_VAL(opt, fName, "");
                if (fName == "")
                {
                    fName = GenerateID(filter.objectID);
                    sv.Option(weoID, fName);
                }
                fName += ".obj";
            }
            fp = fs::path(db_dir) / sv.objectID;
            FileSave(fp, fName, sv);
            retval++;
        }
    }
    else {
        // insert
        LOG4CXX_TRACE(iLogger::GetLogger(), "FsStorage::Set(Record, Record) - insert type=" << data.objectID);
        sv.objectID = data.objectID;
        sv.CopyOptions(&data);
        opt = sv.Option(weoID);
        SAFE_GET_OPTION_VAL(opt, fName, "");
        if (fName == "")
        {
            fName = GenerateID(data.objectID);
            sv.Option(weoID, fName);
        }
        fName += ".obj";
        fp = fs::path(db_dir) / sv.objectID;
        FileSave(fp, fName, sv);
        retval++;
    }
    FixStruct(data.objectID, data);

    return retval;
}

int FsStorage::Set(RecordSet& data)
{
    int retval;
    size_t i;
    Record filt;

    LOG4CXX_TRACE(iLogger::GetLogger(), "FsStorage::Set(RecordSet)");
    filt.Clear();
    retval = 0;
    for (i = 0; i < data.size(); i++)
    {
        filt.objectID = data[i].objectID;
        retval += Set(filt, data[i]);
    }

    return retval;
}

int FsStorage::Delete(Record& filter)
{
    fs::path fp;
    RecordSet* retlist = Search(filter);
    int retval = 0;
    Record del;

    LOG4CXX_TRACE(iLogger::GetLogger(), "FsStorage::Delete");
    for (size_t i = 0; i < retlist->size(); i ++)
    {
        string fName;
        del = (*retlist)[i];
        wOption opt = del.Option("FsStorage::FileName");
        SAFE_GET_OPTION_VAL(opt, fName, "~");
        fp = fs::path(db_dir) / del.objectID;
        FileRemove(fp, fName);
        retval++;
    }
    return retval;
}

RecordSet* FsStorage::Search(Record& filter, bool all/* = false*/)
{
    RecordSet* retlist = new RecordSet;
    fs::path dir_path(db_dir);
    Record* data;
    StringList objProps;
    wOption opt;
    string stValue, stData;

    dir_path /= filter.objectID;
    //dir_path /= "*.obj";
    fs::directory_iterator end_iter;
    fs::directory_iterator dir_itr( dir_path );
    try {
        for ( dir_itr;
            dir_itr != end_iter;
            ++dir_itr )
        {
            if ( fs::is_regular_file( dir_itr->status() ) )
            {
                if (dir_itr->path().extension() == ".obj")
                {
                    data = FileRead(dir_itr->path().string());
                    bool skip = false;
                    if (!all)
                    {
                        objProps = filter.OptionsList();
                        for (size_t j = 0; j < objProps.size(); j++)
                        {
                            opt = filter.Option(objProps[j]);
                            stValue = boost::lexical_cast<std::string>(opt.Value());

                            opt = data->Option(objProps[j]);
                            stData = boost::lexical_cast<std::string>(opt.Value());

                            if (stData != stValue)
                            {
                                skip = true;
                            }
                        } // compare options
                    } // take all files
                    if (!skip)
                    {
                        retlist->push_back(*data);
                        data->objectID = filter.objectID;
                        data->Option("FsStorage::FileName", dir_itr->path().string());
                    }
                    else {
                        delete data;
                    }
                } // *.obj file
            } // regular file
        } // directory iterations
    }
    catch ( const std::exception & e )
    {
        LOG4CXX_ERROR(logger, "FsStorage::Search " << dir_itr->path().string() << " " << e.what());
    }

    return retlist;
}

Record* FsStorage::FileRead(const string& fname)
{
    Record *retval = new Record;

    LOG4CXX_TRACE(logger, "FsStorage::FileRead " << fname);
    try{
        ifstream ifs(fname.c_str());
        boost::archive::text_iarchive ia(ifs);
        // read class instance from archive
        ia >> (*retval);
    }
    catch ( const std::exception & e )
    {
        LOG4CXX_ERROR(logger, "FsStorage::FileRead error: " << e.what());
        delete retval;
        retval = NULL;
    }

    return retval;
}

int FsStorage::FileRemove( const fs::path& fspath, const string& fname )
{
    fs::path fp;
    int retval = 0;

    fp = fspath;
    // remove files
    LOG4CXX_TRACE(logger, "FsStorage::FileRemove " << fp.string() << " --- " << fname);
    if (fname == "*")
    {
        fs::directory_iterator end_iter;
        for ( fs::directory_iterator dir_itr( fp );
            dir_itr != end_iter;
            ++dir_itr )
        {
            try {
                if ( fs::is_regular_file( dir_itr->status() ) )
                {
                    fs::remove(dir_itr->path());
                    retval++;
                }
            }
            catch ( const std::exception & e )
            {
                LOG4CXX_ERROR(logger, "FsStorage::FileRemove " << dir_itr->path().string() << " " << e.what());
            }
        }
    }
    else {
        try {
            fp /= fname;
            fs::remove(fp);
            retval++;
        }
        catch ( const std::exception & e )
        {
            LOG4CXX_ERROR(logger, "FsStorage::FileRemove " << fp.string() << " " << e.what());
        }
    }
    return retval;
}

int FsStorage::FileSave( const fs::path& fspath, const string& fname, const Record& content )
{
    fs::path fp;
    int retval = 0;

    fp = fspath;
    // save data
    LOG4CXX_TRACE(logger, "FsStorage::FileSave " << fp.string() << " --- " << fname);
    fp /= fname;
    try {
        ofstream ofs(fp.string().c_str());
        boost::archive::text_oarchive oa(ofs);
        oa << content;
        retval = 1;
    }
    catch(const std::exception& e)
    {
        LOG4CXX_ERROR(logger, "FsStorage::FileSave can't save " << fp.string() << " : " << e.what());
    }
    return retval;
}

StringList* FsStorage::GetStruct(const string& nspace)
{
    fs::path fp(db_dir);
    StringList *structNames;

    fp /= nspace;
    fp /= "struct";
    structNames = new StringList;
    try
    {
        ifstream is(fp.string().c_str());
        boost::archive::text_iarchive ia(is);
        // read class instance from archive
        ia >> (*structNames);
        // archive and stream closed when destructor are called
    }
    catch(std::exception& e)
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "FsStorage::GetStruct " << nspace << " structure error: " << e.what());
        delete structNames;
        structNames = NULL;
    }

    return structNames;
}

void FsStorage::FixStruct(const string& nspace, Record& strt)
{
    StringMap::iterator obj;
    StringList::iterator lst;
    string stData;
    StringList* structNames;
    StringList flNames;
    size_t i;

    fs::path fp(db_dir);
    fp /= nspace;
    fp /= "struct";

    structNames = GetStruct(nspace);
    if (structNames == NULL)
    {
        structNames = new StringList;
    }
    flNames = strt.OptionsList();
    for (i = 0 ; i < flNames.size(); i++)
    {
        lst = find(structNames->begin(), structNames->end(), flNames[i]);
        if (lst == structNames->end())
        {
            structNames->push_back(flNames[i]);
        }
    }
    try
    {
        ofstream os(fp.string().c_str());
        boost::archive::text_oarchive oa(os);
        // read class instance from archive
        oa << (*structNames);
        // archive and stream closed when destructor are called
    }
    catch(std::exception& e)
    {
        LOG4CXX_WARN(iLogger::GetLogger(), "FsStorage::FixStruct save " << nspace << " structure error: " << e.what());
        return;
    }
}
