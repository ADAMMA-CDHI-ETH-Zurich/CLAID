
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