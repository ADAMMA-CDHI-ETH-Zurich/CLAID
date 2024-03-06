# CLAID - Closing the Loop on AI & Data Collection: Flutter Package
<div align="justify"> 
A pip package containing the Python implementation of CLAID (PyCLAID). CLAID is a flexible and modular middleware framework based on transparent computing. CLAID allows to build applications for mobile (Android, WearOS, iOS*) and regular (Linux, macOS) operating systems, enabling seamless communication between individual Modules implemented in different programming languages (C++, Java, Dart, Python) running on all these operating systems. Existing CLAID Modules allow to effortlessly implement modular machine learning and data collection application with little-to-no coding.  For more details, check out the <a href="https://claid.ethz.ch">CLAID website</a> and our <a href="https://arxiv.org/abs/2310.05643">publication</a>.

CLAID is developed and maintained by the <a href="https://c4dhi.org">Centre for Digital Health Interventions</a> at ETH Zurich. 
</div>



[![DOI](https://img.shields.io/badge/DOI-2310.05643/arxiv.org/abs/2310.05643-red.svg)](https://arxiv.org/abs/2310.05643)
<p align="center">
  <img alt="ETH" src="https://github.com/ADAMMA-CDHI-ETH-Zurich/CLAID/blob/main/assets/eth_logo.png?raw=true" width="45%">
&nbsp; &nbsp; &nbsp; &nbsp;
  <img alt="CDHI" src="https://github.com/ADAMMA-CDHI-ETH-Zurich/CLAID/blob/main/assets/cdhi_logo.png?raw=true" width="45%">
</p>

<p><small style="color: grey;">*iOS support available but not yet released</small></p>


## Features
- Seamless communication between Modules running on different OS or implemented in programming languages, allowing different devices to be integrated into an edge-cloud system
  - Support for Android, WearOS, Linux and macOS (iOS support in the making)
  - Support for C++, Java, Dart and Python*
- Pre-created Modules ready to use without programming, which can be loaded, configured and combined from simple configuration files:
  - Modules for data collection on Android and WearOS
  - Modules for data serialization, storage and upload
  - Modules to execute machine learning models (currently using TensorFlowLite) 
- Background operating via services on Android and WearOS
- Encryption in rest and in-transit (soon) of data sent via a network or stored locally


<p><small style="color: grey;">*pip package will be released separately</small></p>


## Our research
<div align="justify"> 
CLAID is driven by our <strong>Digital Biomarker Research</strong>. In the field of Digital Biomarkers, we use mobile devices like Smartphones, Wearables, and Bluetooth Peripherals to gather datasets for training Machine Learning-based Digital Biomarkers. We observed a lack of tools to repurpose our data collection applications for real-world validation of our research projects. CLAID offers a unified solution for both data collection and integration of trained models, closing a critical gap in Digital Biomarker research.</div>

<div align="justify"> 
If you are interested in our research and how we use CLAID to build mobile AI and Digital Biomarker applications, check out the <a href="https://adamma-cdhi-eth-zurich.github.io">ADAMMA group</a>
(Core for AI & Digital Biomarker, Accoustic and Inflammatory Biomarkers) at the <a href="https://c4dhi.org">Centre for Digital Health Interventions</a>
 at ETH Zurich.</div>

## Source code availability
CLAID is completely open-sourced and released under the Apache2 license. You can access the code from the [CLAID repo](https://github.com/ADAMMA-CDHI-ETH-Zurich/CLAID).

## Issues, Feedback and Contribution
... coming soon

## Contributors
Patrick Langer, ETH Zurich, 2023  
Stephan Altmüller, ETH Zurich, 2023  
Francesco Feher, ETH Zurich, University of Parma, 2023  
Filipe Barata, ETH Zurich, 2023  
