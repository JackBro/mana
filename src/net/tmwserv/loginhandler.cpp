/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "loginhandler.h"

#include "../messagein.h"
#include "protocol.h"

#include "../../logindata.h"
#include "../../main.h"

LoginHandler::LoginHandler()
{
    static const Uint16 _messages[] = {
        APMSG_LOGIN_RESPONSE,
        APMSG_REGISTER_RESPONSE,
        APMSG_RECONNECT_RESPONSE,
        APMSG_PASSWORD_CHANGE_RESPONSE,
        APMSG_EMAIL_CHANGE_RESPONSE,
        0
    };
    handledMessages = _messages;
}

void LoginHandler::setLoginData(LoginData *loginData)
{
    mLoginData = loginData;
}

void LoginHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case APMSG_LOGIN_RESPONSE:
            handleLoginResponse(msg);
            break;
        case APMSG_REGISTER_RESPONSE:
            handleRegisterResponse(msg);
            break;
        case APMSG_RECONNECT_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful login
            if (errMsg == ERRMSG_OK)
            {
                state = STATE_CHAR_SELECT;
            }
            // Login failed
            else
            {
                switch (errMsg) {
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage = "Wrong magic_token";
                        break;
                    case ERRMSG_FAILURE:
                        errorMessage = "Already logged in";
                        break;
                    case LOGIN_SERVER_FULL:
                        errorMessage = "Server is full";
                        break;
                    default:
                        errorMessage = "Unknown error";
                        break;
                }
                state = STATE_ERROR;
            }
        }
            break;

        case APMSG_PASSWORD_CHANGE_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful pass change
            if (errMsg == ERRMSG_OK)
            {
                state = STATE_CHANGEPASSWORD;
            }
            // pass change failed
            else
            {
                switch (errMsg) {
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage = "New password incorrect";
                        break;
                    case ERRMSG_FAILURE:
                        errorMessage = "Old password incorrect";
                        break;
                    case ERRMSG_NO_LOGIN:
                        errorMessage = "Account not connected. Please login first.";
                        break;
                    default:
                        errorMessage = "Unknown error";
                        break;
                }
                state = STATE_ACCOUNTCHANGE_ERROR;
            }
        }
            break;

        case APMSG_EMAIL_CHANGE_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful pass change
            if (errMsg == ERRMSG_OK)
            {
                state = STATE_CHANGEEMAIL;
            }
            // pass change failed
            else
            {
                switch (errMsg) {
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage = "New email address incorrect";
                        break;
                    case ERRMSG_FAILURE:
                        errorMessage = "Old email address incorrect";
                        break;
                    case ERRMSG_NO_LOGIN:
                        errorMessage = "Account not connected. Please login first.";
                        break;
                    case ERRMSG_EMAIL_ALREADY_EXISTS:
                        errorMessage = "The new Email Address already exists.";
                        break;
                    default:
                        errorMessage = "Unknown error";
                        break;
                }
                state = STATE_ACCOUNTCHANGE_ERROR;
            }
        }
            break;

    }
}

void LoginHandler::handleLoginResponse(MessageIn &msg)
{
    const int errMsg = msg.readInt8();

    if (errMsg == ERRMSG_OK)
    {
        readUpdateHost(msg);
        state = STATE_CHAR_SELECT;
    }
    else
    {
        switch (errMsg) {
            case LOGIN_INVALID_VERSION:
                errorMessage = "Client version is too old";
                break;
            case ERRMSG_INVALID_ARGUMENT:
                errorMessage = "Wrong username or password";
                break;
            case ERRMSG_FAILURE:
                errorMessage = "Already logged in";
                break;
            case LOGIN_SERVER_FULL:
                errorMessage = "Server is full";
                break;
            default:
                errorMessage = "Unknown error";
                break;
        }
        state = STATE_LOGIN_ERROR;
    }
}

void LoginHandler::handleRegisterResponse(MessageIn &msg)
{
    const int errMsg = msg.readInt8();

    if (errMsg == ERRMSG_OK)
    {
        readUpdateHost(msg);
        state = STATE_CHAR_SELECT;
    }
    else
    {
        switch (errMsg) {
            case REGISTER_INVALID_VERSION:
                errorMessage = "Client version is too old";
                break;
            case ERRMSG_INVALID_ARGUMENT:
                errorMessage = "Wrong username, password or email address";
                break;
            case REGISTER_EXISTS_USERNAME:
                errorMessage = "Username already exists";
                break;
            case REGISTER_EXISTS_EMAIL:
                errorMessage = "Email address already exists";
                break;
            default:
                errorMessage = "Unknown error";
                break;
        }
        state = STATE_LOGIN_ERROR;
    }
}

void LoginHandler::readUpdateHost(MessageIn &msg)
{
    // Set the update host when included in the message
    if (msg.getUnreadLength() > 0)
    {
        mLoginData->updateHost = msg.readString();
    }
}