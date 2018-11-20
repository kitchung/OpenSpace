/*****************************************************************************************
*                                                                                       *
* OpenSpace                                                                             *
*                                                                                       *
* Copyright (c) 2014-2018                                                               *
*                                                                                       *
* Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
* software and associated documentation files (the "Software"), to deal in the Software *
* without restriction, including without limitation the rights to use, copy, modify,    *
* merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
* permit persons to whom the Software is furnished to do so, subject to the following   *
* conditions:                                                                           *
*                                                                                       *
* The above copyright notice and this permission notice shall be included in all copies *
* or substantial portions of the Software.                                              *
*                                                                                       *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
* PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
* OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
****************************************************************************************/

#include <modules/dsn/managers/signalmanager.h>


namespace openspace {
    constexpr const char* _loggerCat = "SignalManager";

    struct SignalManager::SignalData SignalManager::_signalData;
    std::vector<double> SignalManager::_fileStartTimes;
    std::vector<std::string> SignalManager::_dataFiles;

    bool SignalManager::extractMandatoryInfoFromDictionary(const char* identifier, std::unique_ptr<ghoul::Dictionary> &dictionary)
    {
        bool dataFilesSuccess = JsonHelper::checkFileNames(identifier, dictionary, _dataFiles);
        _fileStartTimes = JsonHelper::getDaysFromFileNames(_dataFiles);
        SignalManager::signalParser(0);

        return dataFilesSuccess;
    }

    bool SignalManager::signalParser(int index) {

        std::string filename;
        if (index == -1 || index > _dataFiles.size())
            return false;

        filename = _dataFiles[index];
        std::ifstream ifs(filename);
        nlohmann::json j = nlohmann::json::parse(ifs);

        SignalManager::Signal structSignal;
        
        std::string startTimeString = JsonHelper::getDayFromFileName(filename);
        const double triggerTime = Time::convertTime(startTimeString);

       _signalData.sequenceStartTime = triggerTime;
       _signalData.signals.clear();
       _signalData.signals.reserve(0);
       // TODO handle the start time/ end time with regard to light time travel
       // ie. This has to be different for uplink and downlink
       // light time travel should extend endtime for uplink, and start time for downlink
      for (const auto& signalsInJson : j["Signals"]) {
          structSignal.dishName = signalsInJson["facility"].get<std::string>();
          structSignal.spacecraft = signalsInJson["projuser"].get<std::string>();
          structSignal.endTime = signalsInJson["eot"].get<std::string>(); 
          structSignal.startTime = signalsInJson["bot"].get<std::string>();
          structSignal.direction = signalsInJson["direction"].get<std::string>();
          structSignal.lightTravelTime = 71397.6659308273;

          //Add signals to vector of signals
          _signalData.signals.push_back(structSignal);
        }

      _signalData.isLoaded = true;
      return _signalData.isLoaded;
    }

}


