#include <emuiibo.hpp>

#define EMU_EMUIIBO_SRV "emuiibo"

namespace emu {

    static Service g_emuiibo_srv;

    bool IsAvailable() {
        auto srv_name = smEncodeName(EMU_EMUIIBO_SRV);
        Handle tmph = 0;
        auto rc = smRegisterService(&tmph, srv_name, false, 1);
        if(R_FAILED(rc)) {
            return true;
        }
        smUnregisterService(srv_name);
        return false;
    }

    Result Initialize() {
        if(serviceIsActive(&g_emuiibo_srv)) {
            return 0;
        }
        return smGetService(&g_emuiibo_srv, EMU_EMUIIBO_SRV);
    }

    void Exit() {
        serviceClose(&g_emuiibo_srv);
    }

    Version GetVersion() {
        Version ver = {};
        serviceDispatchOut(&g_emuiibo_srv, 0, ver);
        return ver;
    }

    void GetVirtualAmiiboDirectory(char *out_path, size_t out_path_size) {
        serviceDispatch(&g_emuiibo_srv, 1,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
            .buffers = { { out_path, out_path_size } },
        );
    }

    EmulationStatus GetEmulationStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_emuiibo_srv, 2, out);
        return static_cast<EmulationStatus>(out);
    }

    void SetEmulationStatus(EmulationStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_emuiibo_srv, 3, in);
    }

    Result GetActiveVirtualAmiibo(VirtualAmiiboData *out_amiibo_data, char *out_path, size_t out_path_size) {
        return serviceDispatch(&g_emuiibo_srv, 4,
            .buffer_attrs = {
                SfBufferAttr_FixedSize | SfBufferAttr_HipcPointer | SfBufferAttr_Out,
                SfBufferAttr_HipcMapAlias | SfBufferAttr_Out,
            },
            .buffers = {
                { out_amiibo_data, sizeof(VirtualAmiiboData) },
                { out_path, out_path_size },
            },
        );
    }

    Result SetActiveVirtualAmiibo(char *path, size_t path_size) {
        return serviceDispatch(&g_emuiibo_srv, 5,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
            .buffers = { { path, path_size } },
        );
    }

    void ResetActiveVirtualAmiibo() {
        serviceDispatch(&g_emuiibo_srv, 6);
    }

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_emuiibo_srv, 7, out);
        return static_cast<VirtualAmiiboStatus>(out);
    }

    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_emuiibo_srv, 8, in);
    }

    void IsApplicationIdIntercepted(u64 app_id, bool *out_intercepted) {
        serviceDispatchInOut(&g_emuiibo_srv, 9, app_id, *out_intercepted);
    }

    void IsCurrentApplicationIdIntercepted(bool *out_intercepted) {
        serviceDispatchOut(&g_emuiibo_srv, 10, *out_intercepted);
    }

    Result TryParseVirtualAmiibo(char *path, size_t path_size, VirtualAmiiboData *out_amiibo_data) {
        return serviceDispatch(&g_emuiibo_srv, 11,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_In,
                SfBufferAttr_FixedSize | SfBufferAttr_HipcPointer | SfBufferAttr_Out,
            },
            .buffers = {
                { path, path_size },
                { out_amiibo_data, sizeof(VirtualAmiiboData) },
            },
        );
    }

}