import Ajax from './ajax'
import AuthManager from './auth-manager'
import URLs from './urls'
import { NotificationManager } from 'react-notifications';

export const Statuses = {
    success: 200,
    created: 201,
    bad_request: 400,
    unauthorized: 401,
    forbidden: 403,
    not_found: 404,
    conflict: 409,
    server_error: 500
};

class SkyPlannerAPI {
    login(username, password, onSuccess, onFailure) {
        Ajax.send_json('/api/users/login', {username: username, password: password}, 'POST')
            .then(Ajax.decode_json({
                is_success: (r) => r.status == Statuses.success,
                success: onSuccess,
                failure: (r) => this.onAPIFailure(r, {[Statuses.unauthorized]: onFailure})
        }));
    } 
    tokenLogin(token, onSuccess, onFailure) {
        Ajax.fetch( URLs.buildAuthPath('/api/users/get'))
            .then(Ajax.decode_json({
                is_success: (r) => r.status == Statuses.success, 
                success: (j) => {
                    Object.assign(j, {token: token});
                    AuthManager.login(j);
                    onSuccess();
                },
                failure: (r) => this.onAPIFailure(r, {[Statuses.unauthorized]: onFailure})
        }));
    }

    register(username, password, onSuccess, onFailure) {
        Ajax.send_json('/api/users/create', {username: username, password: password}, 'PUT')
            .then(Ajax.decode_json({
                is_success: (r) => r.status == Statuses.created,
                success: onSuccess,
                failure: (r) => this.onAPIFailure(r, onFailure)
        }));

    }

    onAPIFailure(response, callbacks) {
        let cb = {
            [Statuses.server_error]: this.serverError,
            [Statuses.unauthorized]: this.unauthorized,
            [Statuses.forbidden]: this.forbidden,
            [Statuses.not_found]: this.notFound
        };
        Object.assign(cb, callbacks);
        if(response.status in cb) {
            cb[response.status](response);
        }
    }

    serverError(response) {
        console.log(response);
        response.text().then( (t) => console.log(t));
        NotificationManager.error('Unexpected server error', 'Error', 5000);
    }
    
    unauthorized(response) {
        console.log(response);
        NotificationManager.warning('Session expired', 'Error', 5000);
        AuthManager.logout();
    }

    forbidden(response) {
        // TODO
        console.log(response);
    }

    notFound(response) {
        // TODO
        console.log(response);
        NotificationManager.error('Resource was not found');
    }
}

export default SkyPlannerAPI;
export let api = new SkyPlannerAPI();
