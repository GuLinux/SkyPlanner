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
    
    static decode_json(response) {
        return response.json();
    }
}

export default Ajax;
