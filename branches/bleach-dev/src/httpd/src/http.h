#include "../../shared/Common.h"
#include "../../shared/Singleton.h"
#include "../../shared/Config/ConfigEnv.h"
#include "../../shared/Database/DatabaseEnv.h"

class Httpd : public Singleton<Httpd>
{
    public:
        Httpd();
        ~Httpd();

		void SetInitialHttpdSettings();

        // update the world server every frame
        void Update(time_t diff);

    private:
        //! Timers
        time_t m_gameTime;
        time_t m_lastTick;

		time_t m_nextThinkTime;
};

#define sHttpd Httpd::getSingleton()
