gpg -ab -u "$gpg_mail" claid-$claid_version.aar 
gpg -ab -u "$gpg_mail" claid-$claid_version.module
gpg -ab -u "$gpg_mail" claid-$claid_version.pom 
gpg -ab -u "$gpg_mail" claid-$claid_version-sources.jar 


md5sum claid-$claid_version.aar  | cut -d " " -f 1 > claid-$claid_version.aar.md5
md5sum claid-$claid_version.module  | cut -d " " -f 1 > claid-$claid_version.aar.module.md5
md5sum claid-$claid_version.pom  | cut -d " " -f 1 > claid-$claid_version.aar.pom.md5
md5sum claid-$claid_version-sources.jar   | cut -d " " -f 1 > claid-$claid_version.aar-sources.jar.md5

sha1sum claid-$claid_version.aar  | cut -d " " -f 1 > claid-$claid_version.aar.sha1
sha1sum claid-$claid_version.module  | cut -d " " -f 1 > claid-$claid_version.aar.module.sha1
sha1sum claid-$claid_version.pom  | cut -d " " -f 1 > claid-$claid_version.aar.pom.sha1
sha1sum claid-$claid_version-sources.jar   | cut -d " " -f 1 > claid-$claid_version.aar-sources.jar.sha1
