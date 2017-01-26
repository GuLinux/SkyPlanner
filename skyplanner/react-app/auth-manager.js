let __instance = null;

class AuthManager {
    constructor() {
        this.observers = [];
        this.__user = null;
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
        AuthManager.instance().observers = AuthManager.instance().observers.filter( (x) => x != observer);
    }

    static setUser(user) {
        AuthManager.instance().__user = user;
        AuthManager.instance().observers.forEach( (o) => o.loginChanged(user) );
    }

    static user() {
        return AuthManager.instance().__user;
    }

    static login(user, remember, nextPath) {
        AuthManager.setUser(user);
        if(remember) {
            localStorage.setItem('user_token', user.token);
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
