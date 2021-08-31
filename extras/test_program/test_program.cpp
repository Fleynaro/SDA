#include <iostream>
#include <Windows.h>

using namespace std;

struct Pos
{
    float x = 0.0f;
    float y = 0.0f;

    __declspec(noinline) float length() const {
        return sqrt(x*x + y*y);
	}
};

enum class EntityType
{
	Ped,
    Player,
	Vehicle
};

class Entity
{
    Pos m_pos;
    EntityType m_type;
public:
    int m_id;
	
    __declspec(noinline) Entity(int id, EntityType type)
	    : m_id(id), m_type(type)
	{
        m_pos = { 1.0f, 1.0f };
    }

    __declspec(noinline) EntityType getType() const {
        return m_type;
    }

    __declspec(noinline) void setPos(Pos pos) {
        m_pos = pos;
    }

    __declspec(noinline) Pos getPos() const {
        return m_pos;
    }

	virtual Pos getWorldPos() {
        return m_pos;
    }
};

class Vehicle;
class Ped : public Entity
{
public:
    Vehicle* m_vehicle = nullptr;
	
    __declspec(noinline) Ped(int id)
	    : Entity(id, EntityType::Ped)
	{}

    __declspec(noinline) void shootAt(const Pos& pos) {
        const auto deltaPos = Pos({ pos.x - getPos().x, pos.y - getPos().y });
        const auto t = deltaPos.length() / 100.0f;
        printf("Ped %i shoots at %.1f, %.1f (t = %.1f)\n", m_id, pos.x, pos.y, t);
    }

    __declspec(noinline) Pos getWorldPos() override;
};

class Vehicle : public Entity
{
public:
    Ped* m_driver;
	
    __declspec(noinline) Vehicle(int id)
        : Entity(id, EntityType::Vehicle)
	{}

    __declspec(noinline) void setDriver(Ped* ped) {
        m_driver = ped;
        ped->m_vehicle = this;
        printf("Ped %i enter the vehicle %i\n", ped->m_id, m_id);
    }
};

Pos Ped::getWorldPos() {
    return Pos({ m_vehicle->getPos().x + getPos().x, m_vehicle->getPos().y + getPos().y });
}

int main()
{
    printf("Hello! This is a test program.\n\n");
    Ped* ped = new Ped(1);
    ped->setPos({ 2.0f, 3.0f });
    Ped* ped_enemy = new Ped(2);
    ped->setPos({ 10.0f, 23.0f });
    Vehicle* car = new Vehicle(1);
    car->setDriver(ped);
    Vehicle* car_enemy = new Vehicle(2);
    car_enemy->setDriver(ped_enemy);
	
    int idx = 1;
    while(true) {
    	if(rand() % 100 < 30)
			ped->shootAt(ped_enemy->getPos());
        if (rand() % 100 < 30)
			ped_enemy->shootAt(ped->getPos());
    	
        printf("out: %i (rand = %i, some value = %i)\n", idx, rand(), idx * idx * 3 - 1);
        idx++;
        Sleep(1000);
    }
	
    return 0;
}