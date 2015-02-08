function getAllPropertyNames(obj) {
    var props = [];

    do {
        Object.getOwnPropertyNames(obj).forEach(function (prop) {
            if (props.indexOf(prop) === -1) {
                props.push(prop);
            }
        });
    } while (obj = Object.getPrototypeOf(obj));

    return props;
}

function raep(elem) {
    var x = 0;
    var y = 0;
    var locking = false;

    function hasLock(elem) {
        return document.pointerLockElement === elem ||
            document.mozPointerLockElement === elem ||
            document.webkitPointerLockElement === elem;
    }

    function lockChangeAlert() {
        locking = false;
        if(document.pointerLockElement ||
            document.mozPointerLockElement ||
            document.webkitPointerLockElement) {
            console.log('The pointer lock status is now locked');
        } else {
            console.log('The pointer lock status is now unlocked');
        }
    }

    function lockError() {
        locking = false;
    }

    document.addEventListener('pointerlockchange', lockChangeAlert, false);
    document.addEventListener('mozpointerlockchange', lockChangeAlert, false);
    document.addEventListener('webkitpointerlockchange', lockChangeAlert, false);

    document.addEventListener('pointerlockerror', lockError, false);
    document.addEventListener('mozpointerlockerror', lockError, false);
    document.addEventListener('webkitpointerlockerror', lockError, false);

    function move(event) {
        var elem = event.target;

        var movementX = event.movementX || event.mozMovementX || event.webkitMovementX || 0;
        var movementY = event.movementY || event.mozMovementY || event.webkitMovementY || 0;

        if(hasLock(elem)) {
            x += movementX;
            y += movementY;
            var rect = elem.getBoundingClientRect();
            x = Math.max(Math.min(x, rect.right), rect.left);
            y = Math.max(Math.min(y, rect.bottom), rect.top);
        } else {
            x = event.pageX;
            y = event.pageY;
        }
    };

    // request pointer lock on each click (unless already have it)
    function clk(event) {
        var elem = event.target;
        if (hasLock(elem) || locking) {
            return;
        }
        locking = true;
        elem.requestPointerLock = elem.requestPointerLock ||
            elem.mozRequestPointerLock ||
            elem.webkitRequestPointerLock;
        elem.requestPointerLock();
    }
    var eventHandlers = {};
    elem.realAddEventListener = elem.addEventListener;
    // this tries to provide a seamless transition between locked and unlocked pointer modes
    elem.addEventListener = function(eventName, handler) {
        if (eventHandlers[eventName] === undefined) {
            eventHandlers[eventName] = [];
            elem.realAddEventListener(eventName, function(event) {
                if (eventName === 'mousemove') {
                    move(event);
                }
                var stubEvent = event;
                if (event.pageX !== undefined) {
                    stubEvent = {};
                    getAllPropertyNames(event).forEach(function(key) {
                        stubEvent[key] = event[key];
                    });
                    stubEvent.preventDefault = function() {event.preventDefault()};
                    stubEvent.stopPropagation = function() {event.stopPropagation()};
                    stubEvent.pageX = x;
                    stubEvent.pageY = y;
                }
                eventHandlers[eventName].forEach(function(handler) {
                    handler(stubEvent);
                })
            });
        }
        eventHandlers[eventName].push(handler);
    }
    elem.addEventListener('click', clk);

    // prevent default actions for F1 and F5 in Chrome/Windows
    document.addEventListener('keydown', function(event) {
        if (event.keyCode >= 112 && event.keyCode <= 123) {
            event.stopPropagation();
            event.preventDefault();
        }
    });
}
