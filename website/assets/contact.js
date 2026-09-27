const contactTriggers = document.querySelectorAll('[data-contact-trigger]');
const contactDialog = document.querySelector('[data-contact-dialog]');
const contactAddress = document.querySelector('[data-contact-address]');
const contactCopy = document.querySelector('[data-contact-copy]');
const contactClose = document.querySelector('[data-contact-close]');
const contactFeedback = document.querySelector('[data-contact-feedback]');
const contactEmail = String.fromCharCode(114, 97, 115, 103, 111, 46, 105, 110, 115, 116, 114, 117, 109, 101, 110, 116, 115, 64, 103, 109, 97, 105, 108, 46, 99, 111, 109);

if (contactTriggers.length && contactDialog && contactAddress) {
  contactTriggers.forEach((trigger) => {
    trigger.addEventListener('click', (event) => {
      event.preventDefault();
      contactAddress.textContent = contactEmail;
      contactFeedback.textContent = '';
      contactDialog.showModal();
    });
  });

  contactClose?.addEventListener('click', () => contactDialog.close());

  contactCopy?.addEventListener('click', async () => {
    try {
      await navigator.clipboard.writeText(contactEmail);
      contactFeedback.textContent = contactFeedback.dataset.copied || 'Email copied.';
    } catch {
      contactFeedback.textContent = contactFeedback.dataset.fallback || 'Select and copy the address above.';
    }
  });
}
