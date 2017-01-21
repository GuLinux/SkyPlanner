let __instance = null;

class LoginDispatcher {
    constructor() {
        this.observers = [];
    }

    static instance() {
        if(! __instance)
            __instance = new LoginDispatcher();
        return __instance;
    }

    static register(observer) {
        LoginDispatcher.instance().observers.push(observer);
    }

    static unregister(observer) {
        LoginDispatcher.instance().observers.pop(observer);
    }

    static setUser(user) {
        LoginDispatcher.instance().observers.forEach( (o) => o.setUser(user) );
    }
}

export default LoginDispatcher;
