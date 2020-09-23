function submitted(){
    let name = document.getElementById('form-name').value;
    let email = document.getElementById('form-email').value;
    let subject = document.getElementById('form-subject').value;
    let message = document.getElementById('form-message').value;

    if(name === ""){
        alert("Field 'name' must not be blank.");
        return;
    }
    if(email === ""){
        alert("Field 'email' must not be blank.");
        return;
    }
    if(subject === ""){
        alert("Field 'subject' must not be blank.");
        return;
    }
    if(message === ""){
        alert("Field 'message' must not be blank.");
        return;
    }


    alert(`Hello, ${name}:\nI'll answer to <${email}> address about '${subject}' as soon as i can.\nThanks for getting in touch!`);
}
