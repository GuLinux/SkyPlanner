let __instance = null;

class AuthManager {
    constructor() {
        this.observers = [];
        this.user = null;
    }

    static instance() {
        if(! __instance)
            __instance = new AuthManager();
        return __instance;
    }

    static register(observer) {
        AuthManager.instance().observers.push(observer);
    }

    static unregister(observer) {
        AuthManager.instance().observers.pop(observer);
    }

    static setUser(user) {
        AuthManager.user = user;
        AuthManager.instance().observers.forEach( (o) => o.loginChanged(user) );
    }

    static user() {
        return AuthManager.instance().user;
    }

    static login(user, remember) {
        AuthManager.setUser(user);
        if(remember) {
            localStorage.setItem('user_token', json.token);
        }
    }

    static logout() {
        AuthManager.setUser(null);
        window.localStorage.removeItem('user_token');
    }

    static token() {
        return localStorage.getItem('user_token');
    }
}

export default AuthManager;
