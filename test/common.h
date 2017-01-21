/* Common basic tests for all SA implementations. */


SECTION("should be initially empty")
{
	REQUIRE(array.begin() == array.end());
}

SECTION("filling should work")
{
	for (int i = 0; i < 10; i++)
	{
		int *el = array.New();
		REQUIRE(el != nullptr);
		*el = i;
	}

	SECTION("New should return nullptr when full")
	{
		REQUIRE(array.New() == nullptr);
	}

	SECTION("iteration should work")
	{
		int i = 0;
		for (int el : array)
			CHECK(el == i++);
		REQUIRE(i == 10);
	}

	SECTION("deletion should work")
	{
		for (int& el : array)
			array.Delete(&el);
		REQUIRE(array.begin() == array.end());
	}

	SECTION("after deletion, insertion should fill the spots")
	{
		int i = 0;
		for (int& el : array)
		{
			if (i++ % 2 == 0)
			{
				array.Delete(&el);
			}
		}
		for (i = 0; i < 10; i += 2)
		{
			*array.New() = i;
		}
		i = 0;
		for (int el : array)
		{
			CHECK(el == i++);
		}
	}
}
