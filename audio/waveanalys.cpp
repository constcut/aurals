#include "waveanalys.h"

#include <vector>
#include <algorithm>

#include <qmath.h>
#include <cmath>


AmplitudeAnalys::AmplitudeAnalys():isAnalysDone(false),middleLine(0)
{

}

bool AmplitudePositionPredicate(const AmplitudeValue& d1, const AmplitudeValue& d2)
{
  return d1.localOffset < d2.localOffset;
}

void AmplitudeAnalys::startAnalys(short *samples, long monoLength)
{
    isAnalysDone = false;

    //stage 1: scaling wave to the size of analys
    //actualy in test stage(now) we are working only with 120 bpm
    //qu in 120 is 2 beats per s, eith - 4, 16th - 8, 32 - 16, 64-32
    //in samples 16th is 44100/8 - 5513, 64th - 1378, 256th - 345
    //scaling to 256th(345 samples) - it would give enought memory eating, and flexibility

    short middleValue = 0;

    scaledWave.clear(); //cleaning last
    rythmicMinSequence.clear();
    rythmicMaxSequence.clear();
    rythmicSplit.clear();

    //int coef = 1; //UNUSED
    int windowWidth = 690*2;

    scaledWaveElement buferElement;

    for (long i = 0; i < monoLength-windowWidth; i+=windowWidth)
    {
        short *currentArea = &samples[i];

        short upperWavePart = -32766;
        short lowerWavePart = +32766;
        short upperPosition = -1;
        short lowerPosition = -1;

        for (short j = 0; j < windowWidth; j++)
        {
            if (currentArea[j] > upperWavePart)
            {
                upperWavePart = currentArea[j];
                upperPosition = j;
            }
            if (currentArea[j] < lowerWavePart)
            {
                lowerWavePart = currentArea[j];
                lowerPosition = j;
            }
            middleValue = (middleValue+currentArea[j])/2;
        }

        buferElement.upper = upperWavePart;
        buferElement.lower = lowerWavePart;
        buferElement.upperLocal = upperPosition;
        buferElement.lowerLocal = lowerPosition;
        buferElement.energy = abs(upperWavePart)+abs(lowerWavePart);



        scaledWave.push_back(buferElement);
    }

    //stage 2: searching for local min and max in space of 1/16
    //because we now scaled to 128th so every 8 values is width of 16th note





    //RythmValue singleNote; //unused
    //a point - attack, b point - attenuation,
    //z not found, s0,s1 not found(later?)

    middleLine = 0;

    cursepathMax:
    long maxValue = 50;//minimal lvl
    long maxPosition = -1;


    for (unsigned long i = 1; i < scaledWave.size()-1 ; ++i)
    {
        //�������� �������� �����
        if (scaledWave[i].energy > 1000) middleLine = (middleLine+scaledWave[i].energy)/2;
        if(scaledWave[i].energy > maxValue)
        {
            for (size_t j = 0; j < rythmicMaxSequence.size(); ++j)
                if (abs(int(rythmicMaxSequence[j].localOffset - i))<=2)//32&&&&&&
                    goto xBreakMax;


                if((scaledWave[i].energy > scaledWave[i-1].energy)&&
                    (scaledWave[i].energy > scaledWave[i+1].energy))
                {
                   long peaking = ((scaledWave[i].energy - scaledWave[i+1].energy)
                                   +(scaledWave[i].energy - scaledWave[i-1].energy))/2;


                   if (rythmicMaxSequence.size() <= 0)
                   {
                       if (scaledWave[i].energy > maxValue)
                       {
                           maxValue =scaledWave[i].energy;
                           maxPosition = i;
                       }
                   }else
                       if (peaking > scaledWave[i].energy/14)
                       {//rythmicSequence[0].attack.peakSize
                           if (scaledWave[i].energy > maxValue)
                           {
                               maxValue =scaledWave[i].energy;
                               maxPosition = i;
                           }
                       }
                }
            }

        xBreakMax:
        ;
    }

    if (maxPosition != -1)
    {
        AmplitudeValue av;

        av.peakSize = maxValue;
        av.localOffset = maxPosition;
        av.stepsToNextMax = 0; //is max
        av.stepsFromLocalMin = -1;

        rythmicMaxSequence.push_back(av);


        if (rythmicMaxSequence.size() < 100)
               goto cursepathMax;
    }


    ///MAXIMUM SEARCH finished O(n) - 100 - minim - posible peaks
/////////////////////////////////////////////////////////
/////////////////SEARCH FOR MINIS

cursepathMin:
long minValue = 32000;
long minPosition = -1;



for (unsigned long i = 1; i < scaledWave.size()-1 ; ++i)
{    if(scaledWave[i].energy < minValue)
    {
        for (size_t j = 0; j < rythmicMinSequence.size(); ++j)
            if (abs(int(rythmicMinSequence[j].localOffset - i))<=2)//32&&&&&&
                goto xBreakMin;


            if((scaledWave[i].energy < scaledWave[i-1].energy)&&
                (scaledWave[i].energy < scaledWave[i+1].energy))
            {
               long peaking = (( scaledWave[i+1].energy -scaledWave[i].energy )
                               +(scaledWave[i-1].energy - scaledWave[i].energy ))/2;


               if (rythmicMinSequence.size() <= 0)
               {
                       minValue =scaledWave[i].energy;
                       minPosition = i;

               }else
                   if (peaking > 0)//scaledWave[i].energy/28
                   {//rythmicSequence[0].attack.peakSize

                           minValue =scaledWave[i].energy;
                           minPosition = i;
                   }
            }
        }

    xBreakMin:
    ;
}

if (minPosition != -1)
{
    AmplitudeValue av;
    av.peakSize = minValue;
    av.localOffset = minPosition;
    av.stepsFromLocalMin = 0; //is min
    av.stepsToNextMax = -1;

    rythmicMinSequence.push_back(av);


    if (rythmicMinSequence.size() < 100)
           goto cursepathMin;
}
/////////////////////////////////////////////
////MINIMALS FOUND ALSO!


//cleaning pseudo max-s
int upperBridge = rythmicMaxSequence.size();
for (int maxI = 0; maxI < upperBridge; ++maxI)
{
    if (rythmicMaxSequence[maxI].peakSize < middleLine)
    {
        rythmicMaxSequence.erase(rythmicMaxSequence.begin()+maxI);
        --upperBridge;
        --maxI;
    }
}


//cleaning pseudo mins-s

//very lowlowlowlow


///////



for (size_t i = 0; i < rythmicMaxSequence.size(); ++i)
    rythmicSplit.push_back(rythmicMaxSequence[i]);

for (size_t i = 0; i < rythmicMinSequence.size(); ++i)
    rythmicSplit.push_back(rythmicMinSequence[i]);

std::sort(rythmicSplit.begin(),rythmicSplit.end(),AmplitudePositionPredicate);

isAnalysDone = true;


int upperSplitBridge = rythmicSplit.size();
for (size_t i =0; i < upperSplitBridge-1; ++i)
{
    if (rythmicSplit[i].stepsFromLocalMin == rythmicSplit[i+1].stepsFromLocalMin)
    {
        if (rythmicSplit[i].stepsFromLocalMin == 0)//minis
        {
            rythmicSplit.erase(rythmicSplit.begin()+i);
           //--i
            --upperSplitBridge;
        }
        if (rythmicSplit[i].stepsFromLocalMin == -1)//maxus
        {
             rythmicSplit.erase(rythmicSplit.begin()+i+1);
            // --i;
             --upperSplitBridge;
        }
    }
}

while (rythmicSplit[0].stepsFromLocalMin == 0)
    rythmicSplit.erase(rythmicSplit.begin());


while (rythmicSplit[rythmicSplit.size()-1].stepsFromLocalMin == rythmicSplit[rythmicSplit.size()-2].stepsFromLocalMin)
    rythmicSplit.erase(rythmicSplit.begin()+(rythmicSplit.size()-1));

    isAnalysDone = true;
}

AmplitudeValue AmplitudeAnalys::searchAttackStart(AmplitudeValue a)
{
  AmplitudeValue rezult = {0};

  //searching in 5-50 ms
  //1 ms is 44.1 samples
  //5 ms is over 221 samples
  //10 ms is 441 samples
  //50 ms is 2205 samples

  //but in our scaled apmlitude energy 1 point means 1378 samples
  //so we search only in 2-3 points before
  //if it would be nesesureuuaierhfsfhk can be used unscaled wave

  int minimum = scaledWave[a.localOffset].energy;
  int posOf = 0;
  for (size_t i = 1; i < 3; ++i)
    if (minimum > scaledWave[a.localOffset-i].energy)
    {
        minimum = scaledWave[a.localOffset-i].energy;
        posOf = i;
    }

  rezult.localOffset = a.localOffset - posOf;
  rezult.peakSize = scaledWave[a.localOffset-posOf].energy;
  rezult.stepsFromLocalMin = 0;
  rezult.stepsToNextMax = -1;

  return rezult;
}

bool AmplitudeAnalys::searchSustains(RythmValue &rv)
{
    const AmplitudeValue a = rv.attack;
    const AmplitudeValue b = rv.attenuation;
    //AmplitudeValue s0,s1;

    //long distance = b.localOffset - a.localOffset;
    long energyDorp = scaledWave[a.localOffset].energy-scaledWave[b.localOffset].energy;


    int chained = 0;
    int chStart = -1;
    int longestChain = -1;


    for (long i = a.localOffset; i < b.localOffset; ++i)
    {
        if (abs(scaledWave[i].energy-scaledWave[i+1].energy) < energyDorp/5)
        {
            //e = scaledWave[i].energy;
            if (chained == 0)
              chStart  = i;
            ++chained;
        }
        else
        {
            if (chained)
            {
                if (longestChain < chained)
                {
                    rv.sustainBegining.localOffset = chStart;
                    rv.sustainBegining.peakSize = scaledWave[chStart].energy;
                    rv.sustainBegining.stepsFromLocalMin = -1;
                    rv.sustainBegining.stepsToNextMax = 0;
                    rv.sustainEndining.localOffset = chStart+chained;
                    rv.sustainEndining.peakSize = scaledWave[chStart+chained].energy;
                    rv.sustainBegining.stepsFromLocalMin = 0;
                    rv.sustainBegining.stepsToNextMax = -1;
                    longestChain = chained;
                }
            }
            chained = 0;
            chStart = -1;
        }
    }

    {
        rv.sustainBegining.localOffset = rv.attack.localOffset+1;
        rv.sustainBegining.peakSize = scaledWave[rv.attack.localOffset+1].energy;
        rv.sustainBegining.stepsFromLocalMin = -1;
        rv.sustainBegining.stepsToNextMax = 0;
        rv.sustainEndining.localOffset = rv.attenuation.localOffset-1;
        rv.sustainEndining.peakSize = scaledWave[rv.attenuation.localOffset-1].energy;
        rv.sustainBegining.stepsFromLocalMin = 0;
        rv.sustainBegining.stepsToNextMax = -1;
    }


    if (longestChain > 3)
        return true;


    return false;
}


void AmplitudeAnalys::calculateNotes()
{
    rezultSequence.clear();


    for (size_t i = 0; i <rythmicSplit.size()/2; i++)
    {
        if (i*2+1 > rythmicSplit.size())
        {
            //misiing
            RythmValue newNote;
            newNote.attack = rythmicSplit[i*2];
            newNote.attenuation = rythmicSplit[i*2];
            newNote.attackBegining = searchAttackStart(newNote.attack);
            rezultSequence.push_back(newNote);
        }
        else
        {
            RythmValue newNote;
            newNote.attack = rythmicSplit[i*2];
            newNote.attenuation= rythmicSplit[i*2+1];
            newNote.attackBegining = searchAttackStart(newNote.attack);
            searchSustains(newNote);
            rezultSequence.push_back(newNote);
        }
    }
}

bool AmplitudeAnalys::isDone()
{
    return isAnalysDone;
}

std::vector<RythmValue> *AmplitudeAnalys::accessRythmSequence()
{
    return 0;
}


void AmplitudeAnalys::cleanUseless()
{
    //DONT USE NOW, DEBUG PAINTING WILL CRUSH
    this->rythmicMaxSequence.clear();
    this->rythmicMinSequence.clear();
    this->rythmicSplit.clear();
    this->scaledWave.clear();
}

void AmplitudeAnalys::fullClean()
{
    this->rezultSequence.clear();
}

//MAY comes

int signMF(long mass)
{
    const long signLowest = 110011;
    const long signLow = 340034;
    const long signMiddle = 770077;
    const long signHigh = 1000000;
    const long signHighest = 1400000;




    if (mass < signLowest) return 0;
    else if (mass < signLow) return 1;
    else if (mass < signMiddle) return 2;
    else if (mass < signHigh) return 3;
    else if (mass < signHighest) return 4;

    return 7;
}

 void AmplitudeAnalys::calculateSilence(short *samples, long monoLength)
 {


     int sectionN = 0;

     const int widthOfMiniFrame = 690;//was 688

     std::vector<long> pusher;

     //stage 0 calculating scores
     while ((sectionN+1)*widthOfMiniFrame < monoLength)
     {
         size_t innerCycle = 0;
         long mass = 0;
         long shape = 0;
         while (innerCycle < widthOfMiniFrame)
         {
             mass += abs(samples[innerCycle+sectionN*widthOfMiniFrame]);
            shape += samples[innerCycle+++sectionN*widthOfMiniFrame];
         }

         pusher.push_back(mass);
         //pusher.push_back(shape);
         ++sectionN;
     }


     //divide into sausage

     for (size_t i = 0; i < pusher.size(); ++i)
     {
        int state =  signMF(pusher[i]);

        if (state < 2)
          ss.pushToSilent(state,i);
        else
         ss.pushToSound(state,i);
     }

     //NOT divided well ((

     for (size_t i = 0; i < ss.sounds.size(); ++i)
     {
        long start = ss.sounds[i].start;
        int amount = ss.sounds[i].amount;

        if (amount > 2)
        candiPeaks.push_back(start); //start of sausage

      if (amount > 2)
        candiStops.push_back(start+amount-1); //end of sausage

      //searching for maximus

     }

 }

 //Harmonical


 FreqTable HarmonicAnalys::freqTab = FreqTable();

 HarmonicAnalys::HarmonicAnalys():
 ampPtr(0)
 {
     knockOne =0;
 }


 FreqTable LocalFreqTable::LFTvote::ft = FreqTable();


 bool PeakFreqPredicate(const Peak& p1, const Peak& p2)
 {
   const double p1f = ((Peak)(p1)).getFreq(); ///WHY!!
   const double p2f = ((Peak)(p2)).getFreq();
   return p1f < p2f;
 }

 void LocalFreqTable::sortPeaks()
 {
     sortagain:
     std::sort(tableElements.begin(),tableElements.end(),PeakFreqPredicate);

     int chainBegin = -1;
     int chainEnd = -1;

     FreqTable aprox;

     for (size_t i =0; i < tableElements.size(); ++i)
     {
         double nextElement = 999.0;

         if ( i < tableElements.size()-1)
            nextElement = tableElements[i+1].getPosition();

         double diffPos = nextElement - tableElements[i].getPosition();
         //qDebug() << "List norm i "<<i<<"; pos "<<tableElements[i].getPosition()<<"; dp "<<diffPos;

         if (diffPos < 1.1)
         {

            if (chainBegin==-1)
            {
                chainBegin = i;
                //qDebug() << "Set as chain begin";
            }
         }
         else
         {
             //qDebug() << "Chain ends - processing then";
             if (chainBegin!=-1)
             {
                 chainEnd = i; //this was a crapp errroooorrr
                 //NOW make only 1 element from a whole chain
                 int maxIndex = -1;
                 double maxAmp = 0.0;

                 for (int j=chainBegin; j<=chainEnd; ++j)
                 {
                     if (tableElements[j].getAmplitude() > maxAmp)
                     {
                         maxIndex = j;
                         maxAmp = tableElements[j].getAmplitude();
                     }

                    //qDebug() << tableElements[j].getPosition() <<";";
                 }

                 //shouldn't change many things
                 //if ((maxIndex != chainBegin) && (maxIndex != chainEnd))
                 {
                    //next step - build triade  L MAX R

                     double leftSumm = 0.0;
                     for (int j=chainBegin; j < maxIndex; ++j)
                         leftSumm += tableElements[j].getAmplitude();

                     double rightSumm = 0.0;
                     for (int j=maxIndex+1; j <= chainEnd; ++j)
                         rightSumm += tableElements[j].getAmplitude();

                     double fullSumm = leftSumm + rightSumm + tableElements[maxIndex].getAmplitude();

                     double leftPart = leftSumm/fullSumm;
                     double middlePart = tableElements[maxIndex].getAmplitude()/fullSumm;
                     double rightPart = rightSumm/fullSumm;

                     //deb
                     double preFullSummLeft = leftSumm + tableElements[maxIndex].getAmplitude();
                     double preFullSummRight = rightSumm + tableElements[maxIndex].getAmplitude();

                     double coL1 = leftSumm/preFullSummLeft;
                     double coL2 = tableElements[maxIndex].getAmplitude()/preFullSummLeft;

                     double coR1 = rightSumm/preFullSummRight;
                     double coR2 = tableElements[maxIndex].getAmplitude()/preFullSummRight;
                     //deb

                     tableElements[maxIndex].amplitude = fullSumm;

                     if (leftPart > rightPart)
                     {
                        tableElements[maxIndex].position -= coL1;

                        double fr = tableElements[maxIndex].position*7.8125; //std coef from 1024 on 8khz
                        tableElements[maxIndex].freq = aprox.getNearest(fr);
                     }
                     if (rightPart > leftPart)
                     {
                        tableElements[maxIndex].position -= coR1;

                        double fr = tableElements[maxIndex].position*7.8125; //std coef from 1024 on 8khz
                        tableElements[maxIndex].freq = aprox.getNearest(fr);
                     }

                     //clean up
                     for (int j=chainEnd; j >= maxIndex+1; --j)
                         tableElements.erase(tableElements.begin()+j);

                     for (int j=chainBegin; j < maxIndex; ++j)
                         tableElements.erase(tableElements.begin()+chainBegin);

                     if ((chainEnd==maxIndex)
                         && (chainBegin==maxIndex))
                     {
                      chainBegin=-1;
                     }
                     else
                      goto sortagain;

                 }
                 /*
                 else
                 {
                     //semichain - or error
                     this->getRezultVotes();
                 }*/


                 chainBegin=-1;
             }
         }
     }

     if (chainBegin!=-1)
     {
        //chain end is a last element then
     }
 }

 /* does work not the best yet
  *
     for (size_t i =0; i < tableElements.size()-1; ++i)
     {
         if (tableElements[i+1].getPosition() - tableElements[i].getPosition() < 1.1)
         {
             double massAmplitude = tableElements[i].getAmplitude() + tableElements[i+1].getAmplitude();

             double firstAmp = tableElements[i].getAmplitude();
             double secondAmp = tableElements[i+1].getAmplitude();

             double secondCoef = secondAmp/massAmplitude;

             tableElements[i].position += secondCoef;
             tableElements[i].amplitude = massAmplitude;

             double fr = tableElements[i].position*7.8125; //std coef from 1024 on 8khz
             tableElements[i].freq = fr; //tableElements[i].freqTab.getNearest(fr);

             if (i < tableElements-3)
             { //group by 4
                 if (tableElements)
             }

             tableElements.erase(tableElements.begin()+i+1);
              goto sortagain;
         }
     }

  */

 void LocalFreqTable::addPeak(Peak &p)
 {
     tableElements.push_back(p);
 }

 void LocalFreqTable::addPeaks(std::vector<Peak> *peaks)
 {
     for (size_t i = 0; i < peaks->size(); ++i)
         tableElements.push_back((*peaks)[i]);

 }

 std::vector<LocalFreqTable::LFTvote> *LocalFreqTable::getRezultVotes()
 {
     return &votes;
 }

 std::vector<Peak> *LocalFreqTable::getPeaks()
 {
     return &tableElements;
 }

 void LocalFreqTable::voteFor(double freq, double voteValue)
 {

     for (size_t i = 0; i < votes.size(); ++i)
     {
         double tF = votes[i].freq;
         double delta = fabs(tF-freq);
         if (delta < 20.0*freq/100.0)
         {
             votes[i].addValue(voteValue);
             return;
         }
     }

     //not found

     LocalFreqTable::LFTvote newCandidate;
     newCandidate.freq = freq;
     newCandidate.value = voteValue;
     votes.push_back(newCandidate);
 }

 void LocalFreqTable::voteForNew(double freq, double voteValue)
 {
     for (size_t i = 0; i < votes.size(); ++i)
     {
         if (votes[i].freq == freq)
         {
             votes[i].addValue(voteValue);
             return;
         }
     }

     LocalFreqTable::LFTvote newCandidate;
     newCandidate.freq = freq;
     newCandidate.value = voteValue;
     votes.push_back(newCandidate);
 }

 bool LocalFreqVotePredicate (const LocalFreqTable::LFTvote &a,const LocalFreqTable::LFTvote &b)
 {
     return a.value > b.value;
 }


 void LocalFreqTable::voteNewWay()
 {
     votes.clear();

     FreqTable freqTable;

     for (size_t i = 0; i < tableElements.size(); ++i)
     {
         double vote = tableElements[i].getAmplitude();
         vote /= 100.0;

         double freq1 = tableElements[i].getFreq();
         double freq2 = freqTable.getNearest(tableElements[i].getFreq()/2 ); //to lower border
         double freq3 = freqTable.getNearest(tableElements[i].getFreq()/3 );
         double freq4 = freqTable.getNearest(tableElements[i].getFreq()/4 );
         double freq5 = freqTable.getNearest(tableElements[i].getFreq()/5 );

         voteForNew(freq1,vote*20.0); //20 25
         voteForNew(freq2,vote*20.0); //20 23
         voteForNew(freq3,vote*20.0); //20 20
         voteForNew(freq4,vote*10.0); //20 16
         voteForNew(freq5,vote*10.0); //20 15
     }

     std::sort(votes.begin(),votes.end(),LocalFreqVotePredicate);

     for (size_t i = 0; i < votes.size(); ++i)
         votes[i].takeReal();

 }

 void LocalFreqTable::voteNow()
 {
       //looking into every peaks harm union
       //making vote as 1.0/amount of hunion for each
       //(AS OPTION) aswo multiply every vote on amplitude value(to make it really valued)

     votes.clear();

     for (size_t i = 0; i < tableElements.size(); ++i)
     {
        int massAmmountOfBits = 0;
        short harm = harmUnion[i];

        for (massAmmountOfBits = 0; harm; harm >>=1)
            massAmmountOfBits+= harm & 1;


       ++massAmmountOfBits; //itself
       double vote = 1.0/massAmmountOfBits;
       vote *=  tableElements[i].getAmplitude();
         //could be commented upper
       //vote width same value for each one at first at self
       voteFor(tableElements[i].getFreq(),vote);

       if (harmUnion[i] & 0x2) voteFor(tableElements[i].getFreq()/2.0,vote);
       if (harmUnion[i] & 0x4) voteFor(tableElements[i].getFreq()/3.0,vote);
       if (harmUnion[i] & 0x8) voteFor(tableElements[i].getFreq()/4.0,vote);
       if (harmUnion[i] & 0x10) voteFor(tableElements[i].getFreq()/5.0,vote);
       if (harmUnion[i] & 0x20) voteFor(tableElements[i].getFreq()/6.0,vote);
       if (harmUnion[i] & 0x40) voteFor(tableElements[i].getFreq()/7.0,vote);
       if (harmUnion[i] & 0x80) voteFor(tableElements[i].getFreq()/8.0,vote);
       if (harmUnion[i] & 0x100) voteFor(tableElements[i].getFreq()/9.0,vote);
       if (harmUnion[i] & 0x200) voteFor(tableElements[i].getFreq()/10.0,vote);
       if (harmUnion[i] & 0x400) voteFor(tableElements[i].getFreq()/11.0,vote);
       if (harmUnion[i] & 0x800) voteFor(tableElements[i].getFreq()/12.0,vote);

     }

     //after full votes passage
     //weeee take num 0 and del all of it votes that are less 25%
     //etc - so votes would be cleaner

     std::sort(votes.begin(),votes.end(),LocalFreqVotePredicate);

 //kill)
     for (size_t i = 0; i < votes.size(); ++i)
         votes[i].takeReal();

 }



 void LocalFreqTable::recountHarmonicalUnion()
 {
     harmUnion.clear();
     short int harm = 0;
     harmUnion.push_back(harm);
     //find all crossing in table of elements
     for (size_t i = 1; i < tableElements.size(); ++i)
     {
         harm = 0;
         for (int j = i-1; j >= 0; --j)
         {
             double dx = tableElements[i].getFreq()/tableElements[j].getFreq();
             int nearest = dx;
             double delta = fabs(dx-nearest);

             if (nearest > 15) break;

             if (delta < 0.13)
                 harm |= 1<<(nearest-1);

             if (delta > 0.84)
                 harm |= 1<<(nearest);
             //cheking is dx really looking like *n
             //then harm |= bit*n;
         }
         harmUnion.push_back(harm);
     }
 }



 HarmonicAnalys::HarmonicAnalys(AmplitudeAnalys *amp): knockOne(0)
 {

     connectAmp(amp);
 }

 bool HarmonicAnalys::connectAmp(AmplitudeAnalys *amp)
 {
     this->ampPtr = amp;
     return true;
 }



 void HarmonicAnalys::startAnalys(short int *zeroPoint)
 {

     for (size_t i =0; i < this->ampPtr->rezultSequence.size(); ++i)//++u actually, u know;
     {
         int start = this->ampPtr->rezultSequence[i].attack.localOffset;
         int length = this->ampPtr->rezultSequence[i].attenuation.localOffset-this->ampPtr->rezultSequence[0].attack.localOffset;
         long startInSamples = start;//*1380;
         long lengthInSamples = length;//*1380; //constant?

         if (lengthInSamples >= 8192)
         {
             QSharedPointer<FFT> four(new FFT(8192));

             four->transform(&zeroPoint[startInSamples]);
             four->countDecibels();
             four->countAmplitude();
             four->findPeaks();

             QSharedPointer<LocalFreqTable> localFreq(new LocalFreqTable());
             localFreq->addPeaks(&four->getPeaks());
             localFreq->sortPeaks();
             localFreq->recountHarmonicalUnion();
             localFreq->voteNow();

             analyses.push_back(four);
             votes.push_back(localFreq);
            //ft.getNearest(123.0);
         }
         else
         {
             QSharedPointer<FFT> four(new FFT(4));
             QSharedPointer<LocalFreqTable> localFreq(new LocalFreqTable());


             analyses.push_back(four);
             votes.push_back(localFreq);
         }
         //extra long 8192*2, 4096, 2048, extra conditions -
     }

 }

 void HarmonicAnalys::startTable(short int *zeroPoint)
 {
     //magic works here
     for (size_t i =0; i < 33; ++i)//++u actually, u know;
     {
         //int start = this->ampPtr->rezultSequence[i].attack.localOffset;
         //int length = this->ampPtr->rezultSequence[i].attenuation.localOffset-this->ampPtr->rezultSequence[0].attack.localOffset;
         long startInSamples = i*8192;
         long lengthInSamples = 8192; //constant?

         if (lengthInSamples >= 8192)
         {
             QSharedPointer<FFT> four(new FFT(8192));

             four->transform(&zeroPoint[startInSamples]);
             four->countDecibels();
             four->countAmplitude();
             four->findPeaks();

             QSharedPointer<LocalFreqTable> localFreq(new LocalFreqTable());
             localFreq->addPeaks(&four->getPeaks());
             localFreq->sortPeaks();
             localFreq->recountHarmonicalUnion();
             localFreq->voteNow();

             analyses.push_back(four);
             votes.push_back(localFreq);
            //ft.getNearest(123.0);
         }

     }

 }
