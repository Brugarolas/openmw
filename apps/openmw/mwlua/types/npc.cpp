#include "types.hpp"

#include <components/esm3/loadnpc.hpp>
#include <components/lua/luastate.hpp>

#include "apps/openmw/mwworld/worldmodel.hpp"
#include <apps/openmw/mwbase/environment.hpp>
#include <apps/openmw/mwbase/mechanicsmanager.hpp>
#include <apps/openmw/mwbase/world.hpp>
#include <apps/openmw/mwmechanics/npcstats.hpp>
#include <apps/openmw/mwworld/class.hpp>
#include <apps/openmw/mwworld/esmstore.hpp>

#include "../stats.hpp"

namespace sol
{
    template <>
    struct is_automagical<ESM::NPC> : std::false_type
    {
    };
}

namespace MWLua
{
    void addNpcBindings(sol::table npc, const Context& context)
    {
        addNpcStatsBindings(npc, context);

        addRecordFunctionBinding<ESM::NPC>(npc, context);

        sol::usertype<ESM::NPC> record = context.mLua->sol().new_usertype<ESM::NPC>("ESM3_NPC");
        record[sol::meta_function::to_string]
            = [](const ESM::NPC& rec) { return "ESM3_NPC[" + rec.mId.toDebugString() + "]"; };
        record["id"]
            = sol::readonly_property([](const ESM::NPC& rec) -> std::string { return rec.mId.serializeText(); });
        record["name"] = sol::readonly_property([](const ESM::NPC& rec) -> std::string { return rec.mName; });
        record["race"]
            = sol::readonly_property([](const ESM::NPC& rec) -> std::string { return rec.mRace.serializeText(); });
        record["class"]
            = sol::readonly_property([](const ESM::NPC& rec) -> std::string { return rec.mClass.serializeText(); });
        record["mwscript"]
            = sol::readonly_property([](const ESM::NPC& rec) -> std::string { return rec.mScript.serializeText(); });
        record["hair"]
            = sol::readonly_property([](const ESM::NPC& rec) -> std::string { return rec.mHair.serializeText(); });
        record["baseDisposition"]
            = sol::readonly_property([](const ESM::NPC& rec) -> int { return (int)rec.mNpdt.mDisposition; });
        record["head"]
            = sol::readonly_property([](const ESM::NPC& rec) -> std::string { return rec.mHead.serializeText(); });
        record["isMale"] = sol::readonly_property([](const ESM::NPC& rec) -> bool { return rec.isMale(); });
        record["baseGold"] = sol::readonly_property([](const ESM::NPC& rec) -> int { return rec.mNpdt.mGold; });

        record["servicesOffered"] = sol::readonly_property([](const ESM::NPC& rec) {
            std::vector<std::string> providedServices;
            std::map<int, std::string> serviceNames = { { ESM::NPC::Spells, "Spells" },
                { ESM::NPC::Spellmaking, "Spellmaking" }, { ESM::NPC::Enchanting, "Enchanting" },
                { ESM::NPC::Training, "Training" }, { ESM::NPC::Repair, "Repair" }, { ESM::NPC::AllItems, "Barter" },
                { ESM::NPC::Weapon, "Weapon" }, { ESM::NPC::Armor, "Armor" }, { ESM::NPC::Clothing, "Clothing" },
                { ESM::NPC::Books, "Books" }, { ESM::NPC::Ingredients, "Ingredients" }, { ESM::NPC::Picks, "Picks" },
                { ESM::NPC::Probes, "Probes" }, { ESM::NPC::Lights, "Lights" }, { ESM::NPC::Apparatus, "Apparatus" },
                { ESM::NPC::RepairItem, "RepairItem" }, { ESM::NPC::Misc, "Misc" }, { ESM::NPC::Potions, "Potions" },
                { ESM::NPC::MagicItems, "MagicItems" } };

            int mServices = rec.mAiData.mServices;
            for (const auto& entry : serviceNames)
            {
                if (mServices & entry.first)
                {
                    providedServices.push_back(entry.second);
                }
            }
            if (!rec.getTransport().empty())
                providedServices.push_back("Travel");
            return providedServices;
        });
        // This function is game-specific, in future we should replace it with something more universal.
        npc["isWerewolf"] = [](const Object& o) {
            const MWWorld::Class& cls = o.ptr().getClass();
            if (cls.isNpc())
                return cls.getNpcStats(o.ptr()).isWerewolf();
            else
                throw std::runtime_error("NPC or Player expected");
        };

        npc["getDisposition"] = [](const Object& o, const Object& player) -> int {
            if (player.ptr() != MWBase::Environment::get().getWorld()->getPlayerPtr())
                throw std::runtime_error("The argument must be a player!");
            const MWWorld::Class& cls = o.ptr().getClass();
            if (!cls.isNpc())
                throw std::runtime_error("NPC expected");
            return MWBase::Environment::get().getMechanicsManager()->getDerivedDisposition(o.ptr());
        };
    }
}
