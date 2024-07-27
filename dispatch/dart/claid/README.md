# CLAID - Closing the Loop on AI & Data Collection: Flutter Package
[![DOI](https://img.shields.io/badge/DOI-10.1016/j.future.2024.05.026-blue.svg)](https://doi.org/10.1016/j.future.2024.05.026)

<div align="justify"> 

A Flutter plugin to use the CLAID framework. CLAID is a flexible and modular framework enabling seamless deployment of **machine learning models** and **data collection modules** across various devices and operating systems. Deploy your existing python machine learning pipelines on mobile devices and connect data streams from sensors in a plug-and-play approach using simple configuration files!

<p align="center">
  <img alt="ETH" src="https://github.com/ADAMMA-CDHI-ETH-Zurich/CLAID/blob/main/assets/claid_mockup.png?raw=true" width="75%">
</p>

CLAID allows to build applications for mobile (Android, WearOS, iOS*) and regular (Linux, macOS) operating systems, enabling seamless communication between individual Modules implemented in different programming languages (C++, Dart, Java, Python, Objective-C) running on all these operating systems. Existing CLAID Modules allow to effortlessly implement modular machine learning and data collection applications with little-to-no coding. For more details, check out the <a href="https://claid.ethz.ch">CLAID website</a> and our <a href="https://www.sciencedirect.com/science/article/pii/S0167739X24002589?via%3Dihub">publication</a>.

CLAID is developed and maintained by the <a href="https://c4dhi.org">Centre for Digital Health Interventions</a> at ETH Zurich. 
</div>

<p align="center">
  <img alt="ETH" src="https://github.com/ADAMMA-CDHI-ETH-Zurich/CLAID/blob/main/assets/eth_logo.png?raw=true" width="45%">
&nbsp; &nbsp; &nbsp; &nbsp;
  <img alt="CDHI" src="https://github.com/ADAMMA-CDHI-ETH-Zurich/CLAID/blob/main/assets/cdhi_logo.png?raw=true" width="45%">
</p>

<p><small style="color: grey;">*iOS support available but not yet released</small></p>


## Features
- Seamless deployment of machine learning projects across mobile, edge and cloud devices
  - Deploy your existing ML code in python directly on Android and even WearOS devices!
  - Plug-in native Sensor Modules (accelerometer, location sensors, audio, ...) or external wearables (smartwatches, spirometer, ...) and connect the data streams directly to Python code for analysis!
- Seamless communication between Modules running on different operating systems or implemented in different programming languages, allowing various devices to be integrated into an edge-cloud system
  - Support for Android, WearOS, Linux and macOS (iOS support in the making)
  - Support for C++, Dart, Java, Python and Objective-C
- Pre-created Modules ready to use without programming, which can be loaded, configured and combined from simple configuration files:
  - Modules for data collection on Android, WearOS and iOS
  - Modules for data serialization, storage and upload
  - Modules to execute machine learning models (e.g., using Python directly on-device, or alternatively using TensorFlowLite) 
- Long-running and stable background operation via services on Android and WearOS (30+ days uptime without interrupt on Android 13+)
- Full device management features on Android and WearOS, enabling to control Wifi/Bluetooth from the background (without user intervention), preventing termination and uninstallation of Apps (useful for digital biomarker studies)
- Encryption in rest and in-transit (soon) of data sent via a network or stored locally


<p><small style="color: grey;">*pip package will be released separately</small></p>

## Getting started
Check out the CLAID tutorial series on our [Website](https://claid.ethz.ch)!



## Our research
<div align="justify"> 
CLAID is driven by our <strong>Digital Biomarker Research</strong>. In the field of Digital Biomarkers, we use mobile devices like Smartphones, Wearables, and Bluetooth Peripherals to gather datasets for training Machine Learning-based Digital Biomarkers. We observed a lack of tools to repurpose our data collection applications for real-world validation of our research projects. CLAID offers a unified solution for both data collection and integration of trained models, closing a critical gap in Digital Biomarker research.</div>

<div align="justify"> 
If you are interested in our research and how we use CLAID to build mobile AI and Digital Biomarker applications, check out the <a href="https://adamma-cdhi-eth-zurich.github.io">ADAMMA group</a>
(Core for AI & Digital Biomarker, Accoustic and Inflammatory Biomarkers) at the <a href="https://c4dhi.org">Centre for Digital Health Interventions</a>
 at ETH Zurich.</div>

## Source code availability
CLAID is completely open-sourced and released under the Apache2 license. You can access the code from the [CLAID repo](https://github.com/ADAMMA-CDHI-ETH-Zurich/CLAID).


## Contributors
Patrick Langer, ETH Zurich, 2023  
Stephan Altmüller, ETH Zurich, 2023  
Francesco Feher, ETH Zurich, University of Parma, 2023  
Filipe Barata, ETH Zurich, 2023  
