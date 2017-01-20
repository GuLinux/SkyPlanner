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
        var opts = { is_success: (r) => true, success: (d) => {}  };
        Object.assign(opts, options);
        
        return function(response) {
            if(opts.is_success(response))
                response.json().then(opts.success);
        };
    }
}

export default Ajax;
