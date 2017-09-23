#include <sc2api/sc2_api.h>

#include <iostream>

using namespace sc2;

class Bot : public Agent {
public:
    virtual void OnGameStart() final {
        std::cout << "Hello, World!" << std::endl;
    }

    virtual void OnStep() final {
        const ObservationInterface& obs = *Observation();
        int32_t new_minerals = obs.GetMinerals();
        int32_t new_vespene = obs.GetVespene();

        if (new_minerals != old_minerals_ || new_vespene != old_vespene_) {
            old_minerals_ = new_minerals;
            old_vespene_ = new_vespene;
            //std::cout << new_minerals << " " << new_vespene << std::endl;
        }

        int32_t food_margin = obs.GetFoodCap() / 10 + 1;
        if (obs.GetFoodUsed() >= obs.GetFoodCap() - food_margin) {
            TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT, UNIT_TYPEID::TERRAN_SCV);
        }
    }

    virtual void OnUnitIdle(const Unit& unit) final {
        switch (unit.unit_type.ToType())
        {
        case UNIT_TYPEID::TERRAN_COMMANDCENTER:
            std::cout << "harversters: " << unit.assigned_harvesters << " " << unit.ideal_harvesters << std::endl;
            //if (unit.assigned_harvesters < unit.ideal_harvesters) {
            if (false) {
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SCV);
            }
            break;

        case UNIT_TYPEID::TERRAN_SCV:
            //if (TryBuildRefinery(ABILITY_ID::BUILD_REFINERY, UNIT_TYPEID::TERRAN_SCV)) {
            //    break;
            //}

            //Tag tag;
            //if (FindNearestRefinery(unit.pos, tag) ||
            //    FindNearestMineral(unit.pos, tag)) {
            //    Actions()->UnitCommand(unit, ABILITY_ID::SMART, tag);
            //}

        default:
            break;
        }
    }

    bool TryBuildStructure(ABILITY_ID ability_id, UNIT_TYPEID builder_type) {
        const ObservationInterface& obs = *Observation();

        Unit builder_unit;
        Units units = obs.GetUnits(Unit::Alliance::Self);

        for (const auto& unit : units) {
            if (unit.unit_type.ToType() == builder_type) {
                auto it_order = std::find_if(unit.orders.begin(), unit.orders.end(), [ability_id](const UnitOrder& order) { return order.ability_id == ability_id; });
                if (it_order != unit.orders.end()) {
                    return false;
                }
                builder_unit = unit;
            }
        }

        float rx = builder_unit.pos.x + 15.0 * GetRandomScalar();
        float ry = builder_unit.pos.y + 15.0 * GetRandomScalar();

        Actions()->UnitCommand(builder_unit, ability_id, Point2D(rx, ry));
        return true;
    }

    bool TryBuildRefinery(ABILITY_ID ability_id, UNIT_TYPEID builder_type) {
        const ObservationInterface& obs = *Observation();

        Unit builder_unit;
        Units units = obs.GetUnits(Unit::Alliance::Self);

        for (const auto& unit : units) {
            if (unit.unit_type.ToType() == builder_type) {
                auto it_order = std::find_if(unit.orders.begin(), unit.orders.end(), [ability_id](const UnitOrder& order) { return order.ability_id == ability_id; });
                if (it_order != unit.orders.end()) {
                    return false;
                }
                builder_unit = unit;
            }
        }

        Tag geyser_tag;
        if (FindNearestVespeneGeyser(builder_unit.pos, geyser_tag)) {
            Actions()->UnitCommand(builder_unit, ability_id, geyser_tag);
            return true;
        }
        return false;
    }

    bool FindNearestMineral(const Point2D& pos, Tag& target_tag) {
        const ObservationInterface& obs = *Observation();
        Units units = obs.GetUnits(Unit::Alliance::Neutral);
        float dist = std::numeric_limits<float>::max();
        target_tag = NullTag;

        for (const auto& unit : units) {
            if (unit.unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
                float d = DistanceSquared2D(pos, unit.pos);
                if (d < dist) {
                    dist = d;
                    target_tag = unit.tag;
                }
            }
        }
        return target_tag != NullTag;
    }

    bool FindNearestVespeneGeyser(const Point3D& pos, Tag& target_tag) {
        const ObservationInterface& obs = *Observation();
        Units units = obs.GetUnits(Unit::Alliance::Neutral);
        float dist = std::numeric_limits<float>::max();

        for (const auto& unit : units) {
            if (unit.unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) {
                float d = DistanceSquared3D(pos, unit.pos);
                if (d < dist) {
                    dist = d;
                    target_tag = unit.tag;
                }
            }
        }
        return dist != std::numeric_limits<float>::max();
    }

    bool FindNearestRefinery(const Point2D& pos, Tag& target_tag) {
        const ObservationInterface& obs = *Observation();
        Units units = obs.GetUnits(Unit::Alliance::Neutral);
        float dist = std::numeric_limits<float>::max();
        target_tag = NullTag;

        for (const auto& unit : units) {
            if (unit.unit_type == UNIT_TYPEID::TERRAN_REFINERY) {
                float d = DistanceSquared2D(pos, unit.pos);
                if (d < dist) {
                    dist = d;
                    target_tag = unit.tag;
                    std::cout << unit.assigned_harvesters << " " << unit.ideal_harvesters << std::endl;
                }
            }
        }
        return target_tag != NullTag;
    }

    virtual void OnUnitCreated(const Unit& unit) {
        //unit.buffs
    }

    int32_t old_minerals_ = 0;
    int32_t old_vespene_ = 0;
}; 

int main(int argc, char* argv[]) {

    Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);
    coordinator.SetRealtime(true);

    Bot bot;
    coordinator.SetParticipants({
        CreateParticipant(Race::Terran, &bot),
        CreateComputer(Race::Zerg)
    });

    coordinator.LaunchStarcraft();
    //coordinator.StartGame(sc2::kMapEmpty);
    //coordinator.StartGame(sc2::kMapBelShirVestigeLE);
    coordinator.StartGame("Melee/Flat64.SC2Map");

    while (coordinator.Update()) {
    }

    return 0;
}
