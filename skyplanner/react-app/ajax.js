import 'whatwg-fetch'

class Ajax {
    static fetch(url, options) {
        return fetch(url, options);
    }

    static send_json(url, data, method) {
        return Ajax.fetch(url, {
            method: method,
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });
    }
    
    static decode_json(options) {
        var opts = { ok_statuses: [200], on_error: function(){} };
        Object.assign(opts, options);        
        return function(response) {
            console.log(opts);
            console.log(response);
            return response.json();
        };
    }
}

export default Ajax;
