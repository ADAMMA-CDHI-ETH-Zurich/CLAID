namespace claid
{
	namespace XMLLoader
	{
		template<typename T>
		class RegisterHelper
		{
		public:
			RegisterHelper(std::string name)
			{
				CLAID_RUNTIME->registerLoader<T>(name);
			}
		};
	}
}
