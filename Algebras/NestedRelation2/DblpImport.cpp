/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#include "Algebras/FText/FTextAlgebra.h"
#include <unistd.h>
#include "DblpParser.h"
#include <fstream>
#include "XmlFileReader.h"

#include "NRel.h"
#include "DblpImport.h"
#include "DblpImportLocalInfo.h"

#ifndef MAX_PATH
#define MAX_PATH 2048
#endif

using namespace nr2a;

DblpImport::Info::Info()
{
  name = "dblpimport";
  signature = string("text -> ") + NRel::BasicType();
  signature = string("text x text -> ") + NRel::BasicType();
  syntax = "_ dblpimport[ _ ]";
  meaning = "Import an XML-file containing a dump of the DBLP to "
      "a nested relation. First parameter is the filename of the "
      "dump and the second is a file containing stopwords (one "
      "per line). ";
  example = "let dblp = '/home/user/dblp/dblp.xml' dblp"
      "['/home/user/dblp/stopwords.txt'];";
}


DblpImport::~DblpImport()
{
}

/*
The operator expects two arguments. The first argument is the name of
the file containing the dump of the DBLP, the second argument must
contain a filename to a text file containing one stopword per line.

*/
/*static*/ListExpr DblpImport::MapType(ListExpr args)
{

  if (nl->HasLength(args, 2))
  {
    if (listutils::isSymbol(nl->First(args), FText::BasicType()))
    {
      if (listutils::isSymbol(nl->Second(args), FText::BasicType()))
      {
        ListExpr type = DblpParser::BuildResultType();
        return type;
      }
      else
      {
        return listutils::typeError(
            "The second parameter is expected to be an FText");
      }
    }
    else
    {
      return listutils::typeError(
          "The first parameter is expected to be an FText");
    }
  }
  return listutils::typeError("Expecting two texts as input");
}

ValueMapping DblpImport::functions[] = { DblpImportValue, NULL };

/*static*/int DblpImport::SelectFunction(ListExpr args)
{
  return 0;
}

/*
At first the operator opens two files for reading. The stopword file
is processed by a helper function "ReadStopwords"[2] described lateron.

The XML file is processed by another class, called "DblpParser"[2].

*/
/*static*/int DblpImport::DblpImportValue(Word* args, Word& result,
    int message, Word& local, Supplier s)
{

  if (message == OPEN)
  {
    DblpImportLocalInfo *info = new DblpImportLocalInfo();
    local.addr = info;
    bool error = false;
    FText *arg0 = static_cast<FText*>(args[0].addr);
    string xmlFilename = arg0->GetValue();

    NRel* nrel = (NRel*) (qp->ResultStorage(s).addr);
    result.setAddr(nrel);

    if (access(xmlFilename.c_str(), F_OK) == -1)
    {
      cmsg.otherError("The given XML-file is not existing.");
      error = true;
    }
    if (!error && access(xmlFilename.c_str(), R_OK) == -1)
    {
      cmsg.otherError("The given XML-file is not readable.");
      error = true;
    }

    string workingDirectory = GetWorkingDirectory();
    int resSetDir = chdir(GetXmlFilePath(xmlFilename).c_str());
    if (resSetDir != 0)
    {
      cmsg.otherError("Error determining directory of XML-file.");
      error = true;
    }

    FText *arg1 = static_cast<FText*>(args[1].addr);
    string stopwordsFilename = arg1->GetValue();
    if (!error && access(stopwordsFilename.c_str(), F_OK) == -1)
    {
      cmsg.otherError("The given stopwords-file is not existing.");
      error = true;
    }
    if (!error && access(stopwordsFilename.c_str(), R_OK) == -1)
    {
      cmsg.otherError("The given stopwords-file is not readable.");
      error = true;
    }

    std::set<std::string> *stopwords = new std::set<std::string>();
    if(!error)
    {
      ReadStopwords(stopwordsFilename, stopwords);
    }

    if(!error)
    {
      ProgressInfo progressInfo;
      progressInfo.Card = GetFilesize(xmlFilename.c_str());
      info->base = progressInfo;
      DblpParser *parser = new DblpParser(nrel, stopwords, info);
      XmlFileReader *reader = new XmlFileReader(xmlFilename, parser, info);
      try
      {
        info->UnitReceived();
        int retReadXml = reader->readXmlFile();
        if (retReadXml != XmlFileReader::c_success)
        {
          if (retReadXml == XmlFileReader::c_fileOpenError)
          {
            throw Nr2aException("Error while opening XML-file");
          }
          else if (retReadXml == XmlFileReader::c_processingError)
          {
            throw Nr2aException("\n" + reader->getErrorMessages());
          }
          else
          {
            assert(false);
          }
          error = true;
        }
      } catch (Nr2aParserException e)
      {
        error = true;
        string msg("\nNR2A parser error(s): ");
        msg += e.what();
        cmsg.otherError(msg);
        cmsg.send();
      } catch (Nr2aException e)
      {
        error = true;
        string msg("\nNR2A error(s): ");
        msg += e.what();
        cmsg.otherError(msg);
        cmsg.send();
      } catch (exception e)
      {
        error = true;
        string msg("\nUnexpected error: ");
        msg += e.what();
        cmsg.otherError(msg);
        cmsg.send();
      }

      delete parser;
      delete reader;
    }
    delete stopwords;
    delete info;
    if (error)
    {
      nrel->Clear();
    }
    int resResetDir = chdir(workingDirectory.c_str());
    if (resResetDir != 0)
    {
      cmsg.otherError("Error while resetting working directory.");
      error = true;
    }
  }
  return 0;
}

/*
This function reads a file with stopwords line by line.

*/
void DblpImport::ReadStopwords(const string & stopwordsFilename,
    std::set<std::string> *stopwords)
{
  std::ifstream infile(stopwordsFilename.c_str());
  std::string line;
  while (std::getline(infile, line))
  {
    stopwords->insert(line);
  }
  infile.close();
}

/*
Returns a file's size in bytes.

*/
/*static*/ std::ifstream::pos_type
DblpImport::GetFilesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

/*
Returns the path of the directory the given file resides in.

*/
/*static*/ string
DblpImport::GetXmlFilePath(const string xmlFilename)
{
  string result = "";
  const string xmlExtension = ".xml";
  if (xmlFilename.size() > xmlExtension.size())
  {
    string end = xmlFilename.substr(xmlFilename.size() - xmlExtension.size());
    if (0 == strncasecmp(end.c_str(), xmlExtension.c_str(),
        xmlExtension.size()))
    {
      int pos = xmlFilename.find_last_of("/\\");
      if (pos >= 0) // Path includes a directory
        result = xmlFilename.substr(0,pos+1);
      else
        result = ""; // Default directory (e.g. ""+"file.ext")
    }
  }
  return result;
}

/*
Returns the applications current working directory.

*/
/*static*/ string
DblpImport::GetWorkingDirectory()
{
  char buffer[MAX_PATH];
  char *pathChar = getcwd(buffer, sizeof(buffer));
  string pathString = "";
  if (pathChar)
  {
    pathString = pathChar;
  }
  return pathString;
}


/*
List of functions for cost estimation.

*/
CreateCostEstimation DblpImport::costEstimators[] =
  { LinearProgressEstimator<DblpImportLocalInfo>::Build };
