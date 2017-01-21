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
		int i = 1;
		for (int el : array)
			CHECK(el == i++);
	}

	SECTION("deletion should work")
	{
		for (int& el : array)
			array.Delete(&el);
		REQUIRE(array.begin() == array.end());
	}
}
