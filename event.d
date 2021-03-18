#!/usr/sbin/dtrace -s

struct tb_event {
	uint8_t type;
	uint8_t mod; /* modifiers to either 'key' or 'ch' below */
	uint16_t key; /* one of the TB_KEY_* constants */
	uint32_t ch; /* unicode character */
	int32_t w;
	int32_t h;
	int32_t x;
	int32_t y;
};

pid$target::tb_peek_event:entry {this->event_ptr=arg0}
pid$target::tb_peek_event:return {
  this->event = (struct tb_event*) copyin(this->event_ptr, sizeof(struct tb_event));
  printf("event: type=%u mod=%u key=%u ch=%lu w=%d h=%d x=%d y=%d\n", this->event->type, this->event->mod, this->event->key, (unsigned long) this->event->ch, this->event->w, this->event->h, this->event->x, this->event->y);
}

