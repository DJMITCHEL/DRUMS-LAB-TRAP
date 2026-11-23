#include "KitManager.h"

namespace mdl
{
    static juce::File appDataDir()
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("MichelDrumLab");
    }

    KitManager::KitManager() : store (appDataDir().getChildFile ("Kits"), ".mdlkit") {}
}
