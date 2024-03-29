/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
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

package adamma.c4dhi.claid.Module;

import java.time.LocalDateTime;

public class ChannelData<T>
{
    private final T data;
    private final LocalDateTime timestamp;
    private final String userToken;

    private final boolean valid;

    public ChannelData(final T data, final LocalDateTime timestamp, final String userToken)
    {
        this.data = data;
        this.timestamp = timestamp;
        this.userToken = userToken;
        this.valid = true;
    }

    public final T getData()
    {
        return this.data;
    }

    public final LocalDateTime getTimestamp()
    {
        return this.timestamp;
    }

    public final String getUserId()
    {
        return this.userToken;
    }

    public boolean isValid()
    {
        return this.valid;
    }

}
