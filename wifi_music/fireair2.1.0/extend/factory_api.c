
#include "factory_api.h"

void factory_set_hook(factory_func_callback *ffc)
{
    /**********************
        driver api
    **********************/
    ffc->driver.led_init =       user_led_ctl;
    ffc->driver.set_status =     user_set_led_status;

    /**********************
        network api
    **********************/
    ffc->network.connect =       staConnect;
    ffc->network.disconnect =    staDisConnect;
    ffc->network.get_mode =      getWifiMode;
    ffc->network.get_status =    check_network_status;
    ffc->network.init =          wifi_init;
    ffc->network.start_ap =      startAp;
    ffc->network.stop_ap =       stopAp;

    /**********************
        player api
    **********************/
    ffc->player.adplayer =           adplayer;
    ffc->player.get_file_context =   adplayer_get_file_context;
    ffc->player.set_file_context =   adplayer_set_file_context;
    ffc->player.tone_play =          tone_start;
    ffc->player.select_ad =          switchAdout;
    ffc->player.get_ad =             getAdout;


    /**********************
        ota update api
    **********************/
    ffc->updator.burn =      flash_burn;
    ffc->updator.check =     ota_check_firm_version;
    ffc->updator.download =  ota_update;
 
}

void initial_module_callback(factory_func_callback *ffc)
{
    net_set_external_callback(ffc);
    ota_set_external_callback(ffc);
#ifdef __MULTI_ROOM__   
    player_set_external_callback(ffc);
#endif

    //driver_set_external_callback(ffc);  

}

void factory_init(factory_func_callback *ffc)
{
    factory_set_hook(ffc);
    
    initial_module_callback(ffc);
}


