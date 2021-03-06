#pragma once
#include "ITeamClassifier.h"

namespace TeamClassify
{
	enum class TeamClassifierTypes
	{
		Dummy,
		TeamClassifier
	};

	/**
	 * @brief Factory for team classifiers.
	 */
	class TeamClassifierFactory
	{
	public:
		/**
		 * @brief Creates an team classifier instance.
		 * @param type type or name of the team classifier
		 * @return pointer to instance of team classifier
		 */
		static ITeamClassifier* CreateTeamClassifier(const TeamClassifierTypes type);
	};
}

