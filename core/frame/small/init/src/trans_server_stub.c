/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "trans_server_stub.h"

#include "liteipc_adapter.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_permission.h"
#include "softbus_proxychannel_manager.h"
#include "trans_channel_manager.h"
#include "trans_session_manager.h"

int32_t ServerCreateSessionServer(void *origin, IpcIo *req, IpcIo *reply)
{
    LOG_INFO("create session server ipc server pop");
    if (req == NULL || reply == NULL) {
        LOG_ERR("invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    uint32_t size;
    const char *pkgName = (const char*)IpcIoPopString(req, &size);
    const char *sessionName = (const char *)IpcIoPopString(req, &size);
    int32_t callingUid = GetCallingUid(origin);
    int32_t callingPid = GetCallingPid(origin);
    if (CheckTransPermission(callingUid, callingPid, pkgName, sessionName, ACTION_CREATE) != SOFTBUS_OK) {
        LOG_ERR("ServerCreateSessionServer no permission");
        IpcIoPushInt32(reply, SOFTBUS_PERMISSION_DENIED);
        return SOFTBUS_PERMISSION_DENIED;
    }
    int32_t ret = TransCreateSessionServer(pkgName, sessionName, callingUid, callingPid);
    IpcIoPushInt32(reply, ret);
    return ret;
}

int32_t ServerRemoveSessionServer(void *origin, IpcIo *req, IpcIo *reply)
{
    LOG_INFO("remove session server ipc server pop");
    if (req == NULL || reply == NULL) {
        LOG_ERR("invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    uint32_t size;
    const char *pkgName = (const char*)IpcIoPopString(req, &size);
    const char *sessionName = (const char *)IpcIoPopString(req, &size);
    int32_t callingUid = GetCallingUid(origin);
    int32_t callingPid = GetCallingPid(origin);
    if (CheckTransPermission(callingUid, callingPid, pkgName, sessionName, ACTION_CREATE) != SOFTBUS_OK) {
        LOG_ERR("ServerRemoveSessionServer no permission");
        IpcIoPushInt32(reply, SOFTBUS_PERMISSION_DENIED);
        return SOFTBUS_PERMISSION_DENIED;
    }
    int32_t ret = TransRemoveSessionServer(pkgName, sessionName);
    IpcIoPushInt32(reply, ret);
    return ret;
}

int32_t ServerOpenSession(void *origin, IpcIo *req, IpcIo *reply)
{
    LOG_INFO("open session ipc server pop");
    if (req == NULL || reply == NULL) {
        LOG_ERR("invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    uint32_t size;
    const char *mySessionName = (const char*)IpcIoPopString(req, &size);
    const char *peerSessionName = (const char *)IpcIoPopString(req, &size);
    const char *peerDeviceId = (const char *)IpcIoPopString(req, &size);
    const char *groupId = (const char *)IpcIoPopString(req, &size);
    int32_t flags = IpcIoPopInt32(req);
    int32_t callingUid = GetCallingUid(origin);
    int32_t callingPid = GetCallingPid(origin);
    char pkgName[PKG_NAME_SIZE_MAX];
    if (TransGetPkgNameBySessionName(mySessionName, pkgName, PKG_NAME_SIZE_MAX) != SOFTBUS_OK) {
        LOG_ERR("TransGetPkgNameBySessionName failed");
        IpcIoPushInt32(reply, SOFTBUS_TRANS_PROXY_SEND_CHANNELID_INVALID);
        return SOFTBUS_TRANS_PROXY_SEND_CHANNELID_INVALID;
    }
    if (CheckTransPermission(callingUid, callingPid, pkgName, mySessionName, ACTION_OPEN) != SOFTBUS_OK) {
        LOG_ERR("ServerOpenSession no permission");
        IpcIoPushInt32(reply, SOFTBUS_PERMISSION_DENIED);
        return SOFTBUS_PERMISSION_DENIED;
    }
    int32_t ret = TransOpenSession(mySessionName, peerSessionName, peerDeviceId, groupId, flags);
    IpcIoPushInt32(reply, ret);
    return ret;
}

int32_t ServerCloseChannel(void *origin, IpcIo *req, IpcIo *reply)
{
    LOG_INFO("close channel ipc server pop");
    if (req == NULL || reply == NULL) {
        LOG_ERR("invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    int32_t channelId = IpcIoPopInt32(req);
    int32_t channelType = IpcIoPopInt32(req);
    int32_t callingUid = GetCallingUid(origin);
    int32_t callingPid = GetCallingPid(origin);
    char pkgName[PKG_NAME_SIZE_MAX];
    char sessionName[SESSION_NAME_SIZE_MAX];

    switch (channelType) {
        case CHANNEL_TYPE_PROXY:
            if (TransProxyGetNameByChanId(channelId, pkgName, sessionName,
                PKG_NAME_SIZE_MAX, SESSION_NAME_SIZE_MAX) != SOFTBUS_OK) {
                LOG_ERR("get session name fail");
                IpcIoPushInt32(reply, SOFTBUS_TRANS_PROXY_SEND_CHANNELID_INVALID);
                return SOFTBUS_TRANS_PROXY_SEND_CHANNELID_INVALID;
            }
            break;
        case CHANNEL_TYPE_UDP:
            if (TransUdpGetNameByChanId(channelId, pkgName, sessionName,
                PKG_NAME_SIZE_MAX, SESSION_NAME_SIZE_MAX) != SOFTBUS_OK) {
                LOG_ERR("get session name fail");
                IpcIoPushInt32(reply, SOFTBUS_TRANS_UDP_CLOSE_CHANNELID_INVALID);
                return SOFTBUS_TRANS_UDP_CLOSE_CHANNELID_INVALID;
            }
            break;
        default:
            IpcIoPushInt32(reply, SOFTBUS_TRANS_INVALID_CLOSE_CHANNEL_ID);
            return SOFTBUS_TRANS_INVALID_CLOSE_CHANNEL_ID;
    }

    if (CheckTransPermission(callingUid, callingPid, pkgName, sessionName, ACTION_OPEN) != SOFTBUS_OK) {
        LOG_ERR("ServerCloseChannel no permission");
        IpcIoPushInt32(reply, SOFTBUS_PERMISSION_DENIED);
        return SOFTBUS_PERMISSION_DENIED;
    }

    int32_t ret = TransCloseChannel(channelId, channelType);
    IpcIoPushInt32(reply, ret);
    return ret;
}

int32_t ServerSendSessionMsg(void *origin, IpcIo *req, IpcIo *reply)
{
    LOG_INFO("close channel ipc server pop");
    if (req == NULL || reply == NULL) {
        LOG_ERR("invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    int32_t channelId = IpcIoPopInt32(req);
    int32_t msgType = IpcIoPopInt32(req);
    uint32_t size = 0;
    const void *data = (const void *)IpcIoPopFlatObj(req, &size);
    int32_t callingUid = GetCallingUid(origin);
    int32_t callingPid = GetCallingPid(origin);
    char pkgName[PKG_NAME_SIZE_MAX];
    char sessionName[SESSION_NAME_SIZE_MAX];
    if (TransProxyGetNameByChanId(channelId, pkgName, sessionName,
        PKG_NAME_SIZE_MAX, SESSION_NAME_SIZE_MAX) != SOFTBUS_OK) {
        LOG_ERR("Trans close channel get pkgName by chanId failed");
        IpcIoPushInt32(reply, SOFTBUS_TRANS_PROXY_SEND_CHANNELID_INVALID);
        return SOFTBUS_TRANS_PROXY_SEND_CHANNELID_INVALID;
    }
    if (CheckTransPermission(callingUid, callingPid, pkgName, sessionName, 0) != SOFTBUS_OK) {
        LOG_ERR("ServerSendSessionMsg no permission");
        IpcIoPushInt32(reply, SOFTBUS_PERMISSION_DENIED);
        return SOFTBUS_PERMISSION_DENIED;
    }
    int32_t ret = TransSendMsg(channelId, data, size, msgType);
    IpcIoPushInt32(reply, ret);
    return ret;
}