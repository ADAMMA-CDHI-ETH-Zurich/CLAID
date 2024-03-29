/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/



    // Runnable* runnable;

    // runnablesChannel.get(runnable);

    // if(runnable->isValid())
    // {
    //     if(runnable->catchExceptions)
    //     {
    //         try
    //         {
    //             runnable->run();
    //         }
    //         catch(std::exception& e)
    //         {
    //             runnable->setException(e.what());
    //         }
    //     }
    //     else
    //     {
    //         runnable->run();
    //     }
    // }

    // runnable->wasExecuted = true;
    
    // if(runnable->stopDispatcherAfterThisRunnable)
    // {
    //     // No more runnables will be executed after this one !
    //     this->active = false;
    // }
    
    // if(runnable->deleteAfterRun)
    // {
    //     delete runnable;
    // }

    // // Yield
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));