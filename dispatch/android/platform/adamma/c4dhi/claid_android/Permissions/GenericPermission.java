/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Francesco Feher
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

package adamma.c4dhi.claid_android.Permissions;
import android.content.pm.PackageManager;


// E.g. usage:
// GenericPermission genericPermission = new GenericPermission(new String[]{"android.permission.CAMERA"});
// genericPermission.blockingRequest();
public class GenericPermission extends Permission {
    private static final int GENERIC_REQUEST_CODE = 100;
    private String[] permissionNames;
    private static final String userDialogTitle = "You need to allow permissions";
    private String userDialogBody = "In the following screen you need to allow %s\n" +
            "to use this app. If you can't see the option you need to open Settings->" +
            "Apps->CLAIDDemo->Permissions.";

    public GenericPermission(String[] manifestPermissionNames) {
        this.permissionNames = manifestPermissionNames;
        userDialogBody = String.format(userDialogBody,
                String.join(" and ", manifestPermissionNames));
    }

    @Override
    public boolean isGranted() {
        boolean isGranted = true;
        for (String singlePermission : permissionNames) {
            isGranted = (super.getContext().checkSelfPermission(singlePermission)
                    == PackageManager.PERMISSION_GRANTED) && isGranted;
        }
        return isGranted;
    }

    @Override
    public void blockingRequest() {
        if (!isGranted()) {
            super.startIntentWithExtras(permissionNames, GENERIC_REQUEST_CODE, userDialogTitle, userDialogBody);
            while (!isGranted()) {};
        }
    }
}
