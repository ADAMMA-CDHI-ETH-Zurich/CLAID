verify_hash()
{
   cd tensorflow
   hash=$(git rev-parse --verify HEAD)
   echo $hash

   echo "Verifying correct branch"
   if [ $hash = "d5b57ca93e506df258271ea00fc29cf98383a374" ]
   then
      echo "Branch correct"
   else 
      echo "TensorFlow branch is incorrect. Please checkout branch v2.11.0 in the TensorFlow repository"
      cd ..
      exit 1
   fi
   echo "Continue"
   cd ..
}

verify_hash