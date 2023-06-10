package com.example.androidclaid;

import AndroidPermissions.StoragePermission;
import JavaCLAID.Module;

public class TestModule extends Module
{
    void initialize()
    {
        System.out.println("Hello world frm test module");
        StoragePermission perm = new StoragePermission();
        perm.blockingRequest();
    }
}
